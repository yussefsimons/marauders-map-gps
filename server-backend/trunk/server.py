#!/bin/python
import socket, MySQLdb

ServerIP = "0.0.0.0"
ServerPort = "20002"
RecvBuffer = 40

MySQL_Host = "localhost"
MySQL_DB = "maraudersmap"
MySQL_Table = "tracking"
MySQL_User = "maraudersmap"
MySQL_Pass = ""


def StartMySQL():
    try:
        db = MySQLdb.connect(MySQL_Host, MySQL_User, MySQL_Pass, MySQL_DB)
    except MySQLdb.Error, e:
        print "Error %d: %s" %(e.args[0], e.args[1])
        sys.exit(1)

##
## Main server socket function
##
def Server():
    try:
	s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
	s.bind((ServerIP, ServerPort))
	print "Listening for trackers...."
	s.listen(1)
	conn, addr = s.accept()
	print 'Tracker connected from IP:', addr
    except socket.error:
	if s:
	    s.close()
	print "Opening socket FAILED: "
	cursor.close()
	conn.close()
	db.close()
	sys.exit(1)

    try:
	while 1:
	data = conn.recv(RecvBuffer)
	if not data:break
 
	#Data Format: tID|Lat|Lon|Alt|Speed
	tid,lat,lon,alt,speed = data.split("|")
	print "\nGot Tracker (ID: " + tid + ") Data:"
	print " - Latitude: " + lat
	print " - Longitude: " + lon
	print " - Altitude: " + alt
	print " - Speed: " + speed	 
	lat = float(lat)
	lon = float(lon)
	alt = float(alt)
	speed = float(speed)
	cursor.execute ("""INSERT INTO tracker (id, lat, lon, alt, speed) VALUES (%s,%s,%s,%s,%s)""", (tid,lat,lon,alt,speed)
	db.commit()
    except KeyboardInterrupt:
	ClearDB(cursor,db);
	cursor.close()
	conn.close()
	db.close()




if __name__ == '__main__':
    StartMySQL()
    Server()

