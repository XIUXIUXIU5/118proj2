char* formMsg(DistanceVectorEntry entry)
{
	char* msg = (char *)calloc(1024, sizeof(char));
	strcat(msg,"|");
	strcat(msg, entry.nodeName);

	char *buffer = (char*) calloc(50, sizeof(char));
	sprintf(buffer, "/%hd",entry.cost);
	strcat(msg, buffer);

	if (buffer)
	{
		free(buffer);
	}
	
	buffer = (char*) calloc(50,sizeof(char));
	sprintf(buffer,"/%hd", entry.port);
	strcat(msg, buffer);

	return msg;

}