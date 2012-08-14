#!/usr/bin/python
CONFIG_FILE="server.conf"

import time, shelve, os, sys, random, string, shutil

try:
    config = open(CONFIG_FILE).read().split("\n")
except IOError:
    cwd = raw_input("MaraudersMap can't find the configuration file 'server.conf'. What directory is it located in?\n(e.g. /home/user/marauders-map)\n>>> ")
    os.chdir(cwd)

conf = shelve.open(".config")
conf.clear()
conf.close()
    
def configure(rehash="NO"):
    config = open(CONFIG_FILE).read().split("\n")
    linenumber = 1
    defined = {}
    for line in config:
        if line == "":
            pass
        elif line[0] == "#":
            pass
        else:
            option = line.split()[0]
            if option == "ServerIP":
                if "ServerIP" in defined:
                    print "*** CONFIG PARSE ERROR ON LINE " + str(linenumber) + " ***"
                    print "*** ServerIP already defined, using earlier definition ***"
                else:
                    try:
                        tmp = line.split("=")[1].strip().split("##")[0].strip()
			if tmp == "":
			    print "*** CONFIG PARSE ERROR ON LINE " + str(linenumber) + " ***"
                            print "*** No arguments for option ServerIP, using default (0.0.0.0) ***"
                            defined["ServerIP"] = "0.0.0.0"
			else:
			    defined["ServerIP"] = tmp
                    except IndexError:
                        print "*** CONFIG PARSE ERROR ON LINE " + str(linenumber) + " ***"
                        print "*** No arguments for option ServerIP, using default (0.0.0.0) ***"
                        defined["ServerIP"] = "0.0.0.0"
            elif option == "ServerPort":
                if "ServerPort" in defined:
                    print "*** CONFIG PARSE ERROR ON LINE " + str(linenumber) + " ***"
                    print "*** ServerPort already defined, using earlier definition ***"
                else:    
                    try:
                        tmp = line.split("=")[1].strip().split("##")[0].strip()
			if tmp == "":
			    print "*** CONFIG PARSE ERROR ON LINE " + str(linenumber) + " ***"
                            print "*** No arguments for option IRCServer, using default (20002) ***"
                            defined["ServerPort"] = "20002"
			else:
			    defined["ServerPort"] = tmp
                    except IndexError:
                        print "*** CONFIG PARSE ERROR ON LINE " + str(linenumber) + " ***"
                        print "*** No arguments for option ServerPort, using default (20002) ***"
                        defined["ServerPort"] = "20002"
            elif option == "MySQL_host":
                if "MySQL_host" in defined:
                    print "*** CONFIG PARSE ERROR ON LINE " + str(linenumber) + " ***"
                    print "*** MySQL_host already defined, using earlier definition ***"
                else:
                    try:
                        defined["MySQL_host"] = line.split("=")[1].strip().split("##")[0].strip()
                    except IndexError:
                        print "*** CONFIG PARSE ERROR ON LINE " + str(linenumber) + " ***"
                        print "*** No arguments for option MySQL_host ***"
                        if rehash == "NO":
                            print "*** FATAL: quitting ***"
                        return "Error"
            elif option == "MySQL_passwd":
                if "MySQL_passwd" in defined:
                    print "*** CONFIG PARSE ERROR ON LINE " + str(linenumber) + " ***"
                    print "*** MySQL_passwd already defined, using earlier definition ***"
                else:
                    try:
                        defined["MySQL_passwd"] = line.split("=")[1].strip().split("##")[0].strip()
                    except IndexError:
                        print "*** CONFIG PARSE ERROR ON LINE " + str(linenumber) + " ***"
                        print "*** No arguments for option MySQL_passwd ***"
                        if rehash == "NO":
                            print "*** FATAL: quitting ***"
                        return "Error"
            elif option == "MySQL_user":
                if "MySQL_user" in defined:
                    print "*** CONFIG PARSE ERROR ON LINE " + str(linenumber) + " ***"
                    print "*** MySQL_user already defined, using earlier definition ***"
                else:
                    try:
                        defined["MySQL_user"] = line.split("=")[1].strip().split("##")[0].strip()
                    except IndexError:
                        print "*** CONFIG PARSE ERROR ON LINE " + str(linenumber) + " ***"
                        print "*** No arguments for option MySQL_user ***"
                        if rehash == "NO":
                            print "*** FATAL: quitting ***"
                        return "Error"
            elif option == "MySQL_db":
                if "MySQL_db" in defined:
                    print "*** CONFIG PARSE ERROR ON LINE " + str(linenumber) + " ***"
                    print "*** MySQL_db already defined, using earlier definition ***"
                else:
                    try:
                        defined["MySQL_db"] = line.split("=")[1].strip().split("##")[0].strip()
                    except IndexError:
                        print "*** CONFIG PARSE ERROR ON LINE " + str(linenumber) + " ***"
                        print "*** No arguments for option MySQL_db ***"
                        if rehash == "NO":
                            print "*** FATAL: quitting ***"
                        return "Error"

                    except IndexError:
                        print "*** CONFIG PARSE ERROR ON LINE " + str(linenumber) + " ***"
                        print "*** No arguments for option mod_SiteAdminTools, using default (FALSE) ***"
			defined["mod_SiteAdminTools"] = "FALSE"
	    else:
                print "*** CONFIG PARSE ERROR ON LINE " + str(linenumber) + " ***"
                print "*** Unknown option: " + option + " ***"
                print "*** Ignoring ***"
        linenumber += 1
            
    if "ServerIP" not in defined.keys():
        print "*** CONFIG PARSE ERROR ***"
        print "*** No ServerIP defined ***"
        if rehash == "NO":
            print "*** FATAL: quitting ***"
        return "Error"
    if "ServerPort" not in defined.keys():
        print "*** CONFIG PARSE ERROR ***"
        print "*** No ServerPort defined ***"
        if rehash == "NO":
            print "*** FATAL: quitting ***"
        return "Error"
    if "MySQL_host" not in defined.keys():
        print "*** CONFIG PARSE ERROR ***"
        print "*** No MySQL_host defined ***"
        if rehash == "NO":
            print "*** FATAL: quitting ***"
        return "Error"
    if "MySQL_user" not in defined.keys():
        print "*** CONFIG PARSE ERROR ***"
        print "*** No MySQL_user defined ***"
        if rehash == "NO":
            print "*** FATAL: quitting ***"
        return "Error"
    if "MySQL_passwd" not in defined.keys():
        print "*** CONFIG PARSE ERROR ***"
        print "*** No MySQL_passwd defined ***"
        if rehash == "NO":
            print "*** FATAL: quitting ***"
        return "Error"
    if "MySQL_db" not in defined.keys():
        print "*** CONFIG PARSE ERROR ***"
        print "*** No MySQL_db defined ***"
        if rehash == "NO":
            print "*** FATAL: quitting ***"
        return "Error"

    return defined
