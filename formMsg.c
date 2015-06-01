char* formMsg(mapc_int* dv)
{
	char* msg = (char *)calloc(1024, sizeof(char));
	for(mapc_int::iterator it = (*dv).begin(); it != (*dv).end(); it++)
	{
		strcat(msg,"|");
		strcat(msg, it->first);

		char *buffer = (char*) calloc(50, sizeof(char));
		sprintf(buffer, "/%hd",it->second);
		strcat(msg, buffer);
		if (buffer)
		{
			free(buffer);
		}
	}

	return msg;

}