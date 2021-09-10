#include "Commands_FileIO.h"
#include "Script.h"

#ifdef OBLIVION

#include "GameAPI.h"

/* Return a float value from the given file.
 * syntax: FloatFromFile filename line_number
 * shortname: ffromf
 *
 * Gets a float from the given line from the file [filename], starting with line 0.
 * -If line_number <= 0, get the first line of the file.
 * -If line_number >= (lines_in_file - 1), get the last line of the file.
 *
 * -Filename is relative to the path where Oblivion.exe is located.
 * -Integers can be read from file, but will be returned as floats (range limits on floats may apply).
 * -This function has been tested using the Oblivion console.  More testing might be needed in scripts to make sure the
 *  returned value is correct.
 * -Prints an error to the console and returns nothing if the file is not found.
 */
bool Cmd_FloatFromFile_Execute(COMMAND_ARGS)
{
	*result = 0.0;
	char lineBuffer[BUFSIZ];
	FILE* fileptr;
	int currLine = 0;
	char filename[129];
	int linePos;

	//just return with error message if file can't be opened
	if (!(ExtractArgs(PASS_EXTRACT_ARGS, &filename, &linePos)) ||
		fopen_s(&fileptr, filename, "r"))
	{
		Console_Print ("File %s could not be opened.", filename);
		return true;
	}

	//fgets() will move the file pointer to next line for us, allowing for variable line lengths
	while (fgets (lineBuffer, BUFSIZ, fileptr) && currLine < linePos)
	{
		currLine++;
	}

	*result = (float) atof(lineBuffer);
	//Console_Print ("Line %d: %f", linePos, *result);
	fclose (fileptr);
	return true;
}

/* Write float to specified line in file
 * syntax:  FloatToFile filename line_number float_to_write
 * shortname: ftof
 *
 * Writes the specified floating-point value to the file at the given line number (starting at 0).  This will replace
 * whatever used to be at that line.  This function copies the given file to "temp.txt" and replaces the given line while
 * doing so.  The contents of the temp file are then copied back to the original, updating it.
 * -If line_number is < 0 or > (lines_in_file - 1), the function will not write anything.
 *
 * -Filename is relative to where Oblivion.exe is located.
 * -Integers can be written, but will be written as floats (range limits on floats may apply), so 5 will be written
 *  as 5.00000.
 * -This was tested using the console, with FloatFromFile to verify results.  It should work fine in a script.
 * -Prints error message to console and does nothing if file does not exist.
 */
bool Cmd_FloatToFile_Execute(COMMAND_ARGS)
{
	*result = 0.0;
	char lineBuffer[BUFSIZ];
	FILE* fileptr;
	FILE* tempptr;
	int currLine = 0;
	char filename[128];
	int linePos;
	float input;

	if (!(ExtractArgs(PASS_EXTRACT_ARGS, &filename, &linePos, &input)) ||
		fopen_s(&fileptr, filename, "r"))
	{
		Console_Print ("File %s could not be opened.", filename);
		return true;
	}
	fopen_s (&tempptr, "temp.txt", "w");
	//Console_Print ("input: %f", input);

	//read from original to temp
	while (fgets (lineBuffer, BUFSIZ, fileptr))
	{
		if (currLine == linePos)  //replace line with given input
		{
			sprintf_s (lineBuffer, sizeof(lineBuffer), "%f\n", input);
			//Console_Print("wrote: %s",lineBuffer);
		}
		currLine++;
		fputs (lineBuffer, tempptr);
	}

	//is there a better way to change stream modes?
	fclose (tempptr);
	fclose (fileptr);

	fopen_s (&tempptr, "temp.txt", "r");
	fopen_s (&fileptr, filename, "w");

	//write data back to original file
	while (fgets (lineBuffer, BUFSIZ, tempptr))
	{
		fputs (lineBuffer, fileptr);
	}

	fclose (tempptr);
	fclose (fileptr);

	return true;
}
#endif

static ParamInfo kParams_ReadFromFile[2] =
{
	{"string", kParamType_String,	0},
	{	"int", kParamType_Integer,	0}
};

static ParamInfo kParams_WriteToFile[3] =
{
	{"string", kParamType_String,	0},
	{	"int", kParamType_Integer,	0},
	{ "float", kParamType_Float,	0}
};

CommandInfo kCommandInfo_FloatFromFile =
{
	"FloatFromFile",
	"ffromf",
	0,
	"Read a number from specified line in file",
	0,
	2,
	kParams_ReadFromFile,
	HANDLER(Cmd_FloatFromFile_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_FloatToFile =
{
	"FloatToFile",
	"ftof",
	0,
	"Write a number to specified line in file",
	0,
	3,
	kParams_WriteToFile,
	HANDLER(Cmd_FloatToFile_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};