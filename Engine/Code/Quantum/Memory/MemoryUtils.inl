//-----------------------------------------------------------------------------------------------
// INLINE FILE FOR MEMORYUTILS.H
//-----------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------
template<typename T, typename... Args>
static char* GetBufferFromTemplateArgs(char** memory, const T& data, Args... args)
{

}


//-----------------------------------------------------------------------------------------------
template<typename... Args>
char* Memory::GetVAListFromTemplateArgs(Args... args)
{
	char* result = nullptr;
	return GetBufferFromTemplateArgs(&result, args);
}