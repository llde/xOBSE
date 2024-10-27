#pragma once

#ifdef _M_ARM_
#error Does't support ARM
#endif

#define WIN32_LEAN_AND_MEN
#include <Windows.h>
#include <inttypes.h>
#include <intrin.h>
#include <map>
#include <mutex>

#include "detours.h"
#include "x86Decoder.h"

namespace easydetour_internals { class __easy_detour_Iternal_class; }


namespace EasyDetour
{

	template<typename T, typename R, typename ... Types>
	class EasyDetour
	{
	public:
		typedef R(*TARGET_FUNCTION)(Types...);
		typedef R(T::*DETOUR_CLASS_FUNCTION)(TARGET_FUNCTION, Types ...);
		typedef R(*DETOUR_FUNCTION)(T*, TARGET_FUNCTION, Types ...);

		EasyDetour(T* pClassIntance, TARGET_FUNCTION lpTargetFunction)
			: pInstance(pClassIntance), lpTargetFunction(lpTargetFunction), bitFlag(0)
		{
			this->lpTrampolineFunction = lpTargetFunction;
			lpDetourFunction.lpClassFunction = nullptr;
		}
		~EasyDetour()
		{
			if (isFunHooked())
			{
				easydetour_internals::__easy_detour_Iternal_class::_removeDetourInstance(lpTargetFunction);
				UnHookFunction();
			}
		}

		bool HookFunction(DETOUR_CLASS_FUNCTION lpClassFunction)
		{
			if (isFunHooked())
				return false;
			if (DetourTransactionBegin() == NO_ERROR)
			{
				DetourUpdateThread(GetCurrentThread());
				DetourAttach((LPVOID*)&this->lpTrampolineFunction, (LPVOID)&GenericDetour);
				if (DetourTransactionCommit() == NO_ERROR)
				{
					lpDetourFunction.lpClassFunction = lpClassFunction;
					easydetour_internals::__easy_detour_Iternal_class::_addEasyDetourInstance(this, (LPVOID)this->lpTargetFunction);
					isClassFunction(true);
					isFunHooked(true);
					return true;
				}
				else
					DetourTransactionAbort();
			}
			return false;
		}
		bool HookFunction(DETOUR_FUNCTION lpDetourFunction)
		{
			if (isFunHooked())
				return false;

			if (DetourTransactionBegin() == NO_ERROR)
			{
				DetourUpdateThread(GetCurrentThread());
				DetourAttach((LPVOID*)&this->lpTrampolineFunction, (LPVOID)&GenericDetour);
				if (DetourTransactionCommit() == NO_ERROR)
				{
					this->lpDetourFunction.lpDetourFunction = lpDetourFunction;
					easydetour_internals::__easy_detour_Iternal_class::_addEasyDetourInstance(this, (LPVOID)this->lpTargetFunction);
					isClassFunction(false);
					isFunHooked(true);
					return true;
				}
				else
					DetourTransactionAbort();
			}
			return false;
		}
		bool UnHookFunction()
		{
			if (!isFunHooked())
				return false;

			if (DetourTransactionBegin() == NO_ERROR)
			{
				DetourUpdateThread(GetCurrentThread());
				DetourDetach((LPVOID*)&this->lpTrampolineFunction, (PVOID)&GenericDetour);
				if (DetourTransactionCommit() == NO_ERROR)
				{
					easydetour_internals::__easy_detour_Iternal_class::_removeDetourInstance(lpTargetFunction);
					isFunHooked(false);
					return true;
				}
				else
					DetourTransactionAbort();
			}
			return false;
		}


	private:
		T* const pInstance;
		uint8_t bitFlag;
		const TARGET_FUNCTION lpTargetFunction;
		TARGET_FUNCTION lpTrampolineFunction;

		union
		{
			DETOUR_CLASS_FUNCTION lpClassFunction;
			DETOUR_FUNCTION lpDetourFunction;
		}lpDetourFunction;

		constexpr bool isFunHooked() { return (bool)((bitFlag & 0xF0) > 0x7); }
		constexpr void isFunHooked(bool value)
		{
			if (value)
				bitFlag |= 0xF0;
			else
				bitFlag -= bitFlag & 0xF0;
		}
		constexpr bool isClassFunction() { return (bool)((bitFlag & 0xF) > 0x3); }
		constexpr void isClassFunction(bool value)
		{
			if (value)
				bitFlag |= 0xF;
			else
				bitFlag -= bitFlag & 0xF;
		}

		static R __cdecl GenericDetour(Types ...args)
		{
			const void* stackR = _AddressOfReturnAddress();

			const uintptr_t retAddrs = *(uintptr_t*)stackR;

			void* lpHookFunction;
			EasyDetour<T, R, Types...>* pInstance;

			CallCommand pCallCmd;
			if (decodeCall((void*)retAddrs, &pCallCmd))
			{
				if (pCallCmd.type == NEAR_ABSOLUTE_CALL)
				{
					/*
						============================ ISSUE #1 ============================
						We cant get the function pointer if is store in another register than EBP
						because EBP is saved in every function call.
						To solve this issue we need to create a custom prolog and epilog of this function
						and save the registers in the process.
						EBP is the most likely to be the register used because locals use EBP
						with some offset to be accessed
						==================================================================
					*/
					if (pCallCmd.types.callMPTR.reg == REG_EBP)
					{
						const void* basePointer = (void*)((char*)stackR - sizeof(void*));
						lpHookFunction = (void*)(*(intptr_t*)(*((intptr_t*)basePointer) + pCallCmd.types.callMPTR.offset));
					}
					else
						lpHookFunction = nullptr;
				}
				else
				{
#ifdef _M_AMD64
					const uintptr_t relativePtr = *((uint32_t*)(retAddrs - 0x4)) | 0xffffffff00000000;
#else
					//const uintptr_t relativePtr = *((uintptr_t*)(retAddrs - 0x4));
					const uintptr_t relativePtr = *((uint32_t*)(retAddrs - 0x4));
#endif
					lpHookFunction = (void*)(relativePtr + retAddrs);
				}
			}
			else
				lpHookFunction = nullptr;

			pInstance = reinterpret_cast<EasyDetour<T, R, Types...>*>(easydetour_internals::__easy_detour_Iternal_class::_getDetourInstance(lpHookFunction));

			if (!pInstance) 
			{
				// if We reach here, we got a big problem. But we can't call a null pointer, so is nothing left to do...
				return (R)0;
			}
			if (pInstance->isClassFunction())
			{
				T* const klass = pInstance->pInstance;
				return (*klass.*pInstance->lpDetourFunction.lpClassFunction)(pInstance->lpTrampolineFunction, args...);
			}
			return pInstance->lpDetourFunction.lpDetourFunction(pInstance->pInstance, pInstance->lpTrampolineFunction, args...);
		}
	};

	template<typename T, typename R, typename ...Types>
	EasyDetour<T, R, Types...> make_detour(T* klass, R(*pTargetFunction)(Types...))
	{
		return EasyDetour<T, R, Types...>(klass, pTargetFunction);
	}

}


namespace easydetour_internals {

	class __easy_detour_Iternal_class
	{
	public:
		template<typename T, typename R, typename ...Types>
		friend class EasyDetour;

		static void _addEasyDetourInstance(void* pInstance, void* pDetourTargetFunction);
		static void _removeDetourInstance(void* pDetourTargetFunction);
		static void* _getDetourInstance(void* pDetourTargetFunction);

	private:
		/// <summary>
		/// key-> TargetFunction | value -> easyDetour instance
		/// </summary>
		static std::map<void*, void*> easydetourMap;
		static std::mutex mtx;
	};
}