#include "../lib/json.hpp"
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "../lib/datastructs.h"
using json = nlohmann::json;

/*
 * json j ={
 * {"cmd" = RECV or SEND},
 * {"id" = "id" },
 * {"portno" = n},
 * {"msg" = buf}
 * }
 */

static enum  command
convert_to_enum (const std::string cmd)
{
	// convert the json cmd field into the C enum
	if (cmd == "RECV") {
		return RECV;
	} else if (cmd == "SEND") {
		return SEND;
	} else if (cmd == "EXIT") {
		return EXIT;
	}
}

static  std::string
convert_from_enum (const enum command c)
{
	std::string st;
	switch (c) {
		case RECV :
			st = "RECV";
			break;
		case SEND :
			st = "SEND";
			break;
		case EXIT :
			st = "EXIT";
			break;
	}
	return st;
}

extern "C" struct data_wrapper
convert_string_to_datastruct (const char *jsonCh)
{
	// receive a char sent by another peer and translate that into a datawrapper that contains all the informations
	/*
	 * first translate the string to a json
	 * then populate the datastruct 
	 * dumping the json
	 */
	std::string st (jsonCh); // translate char* to std::string
	if (std::count (st.begin (), st.end (), '}') > 1) {
		st.erase (st.find ('}') + 1, std::string::npos); // it seems that mongoose doesn't clean io->buf, so we truncate the string after the first \{
	}
	printf ("%s\n", jsonCh);
	std::cout << "json.cpp:50: Received: " << st << std::endl;
	auto j = json::parse (st);
	struct data_wrapper data;
	memset (data.id, 0, 30);
	std::string jmsg = j["msg"];
	data.msg = strdup (jmsg.c_str());
	std::string jid = j["id"];
	strncpy (data.id, jid.c_str (), strlen (jid.c_str()));
	data.id[strlen (jid.c_str ()) + 1] = '\0';
	std::cout << strlen (jid.c_str ()) << std::endl;
	data.portno = j["portno"];
	data.cmd = convert_to_enum (j["cmd"]);

	return data;
}

extern "C" char *
convert_datastruct_to_char (const struct data_wrapper data)
{
	// does the opposite,
	// takes a data_wrapper and return a char* to be sent over a socket
	/*
	 * first populate a json
	 * then dump json to string
	 */
	json j;
	j["cmd"] = convert_from_enum (data.cmd);
	j["id"] = data.id;
	j["msg"] = data.msg;
	j["portno"] = data.portno;
	std::cout << j.dump () << std::endl;
	std::string st =  j.dump ();
	return strdup  (st.c_str ());
};
