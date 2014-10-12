#include "pgsampler.h"
/*
 *
 *   These functions include those for establishing and maintaining 
 *   sockets, and sending data over sockets to a monitoring server.
*/


/* 
 * This function establishes a socket connection to the server and executes the handshake command.
 *
 * return code 1: unable to setup basics for socket communication
 * return code 2: unable to establish connection
 *   1 is used by ensure_valid_environment
*/
int establish_connection(void) {
	int conn_result;
	char* data;
	struct sockaddr_in serv_addr;
	struct timeval timeout;      
    
  struct addrinfo hints, *res, *p;
  int status;
  //char ipstr[INET6_ADDRSTRLEN];
  struct in_addr *addr = NULL;
    
  timeout.tv_sec = TIMEOUT_SECS;
  timeout.tv_usec = 0;

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_INET; //  or AF_INET6 AF_UNSPEC to force version
  hints.ai_socktype = SOCK_STREAM;

  /* Resolve pgsampler.io IP address*/
  if ((status = getaddrinfo(output_network_host, NULL, &hints, &res)) != 0) {
      elog(LOG, "getaddrinfo: %s\n", gai_strerror(status));
      return 1;
  }

  for(p = res;p != NULL; p = p->ai_next) {
      if (p->ai_family == AF_INET) { // IPv4
          struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
          addr = &(ipv4->sin_addr);
      } 
      // convert the IP to a string and print it:
      //inet_ntop(p->ai_family, addr, ipstr, sizeof ipstr);
      //elog(LOG, "  %s: %s\n", ipver, ipstr);
  }

  freeaddrinfo(res);

  if(addr == NULL) {
    elog(LOG, "\n Error : Unable to resolve pgsampler.io \n");
    return 1;
  }

	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    elog(LOG, "\n Error : Could not create socket \n");
    return 1;
  }

	memset(&serv_addr, '0', sizeof(serv_addr)); 

	serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(PORT); 
  serv_addr.sin_addr = *addr;
  
	/*if(inet_pton(AF_INET, "0.0.0.0", &serv_addr.sin_addr) <= 0) {
		elog(LOG, "\n inet_pton error occured\n");
		return 1;
	}*/

  if (setsockopt (sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0) {
    elog(LOG, "setsockopt failed\n");
    return 1;
  }

  if (setsockopt (sockfd, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout, sizeof(timeout)) < 0) {
    elog(LOG, "setsockopt failed\n");
    return 1;
	}

	if( (conn_result = connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr))) < 0) {
		return 2;
	}
	
	elog(LOG, "Relational Systems Connection Established.");

  data = handshake();
  // elog(LOG, "HANDSHAKE = %s", data); //Debug info
  if(sockfd != 0)
	  send_data(data);
  pfree(data);

	return 0;
}


/* 
 * This function sends data to the server.
 */
int send_data(char *data) {
  
  int n = 0, conn_result, timeouts_elapsed;
  bool response_received;
  char recvBuff[10];

	memset(recvBuff, '0', sizeof(recvBuff));	
	if (sockfd == 0) {
		conn_result = establish_connection();
		if (conn_result > 0) 
			return 1;	
	}

	// Write data
	// elog(LOG, "\nsending...\n");
	n = write(sockfd, data, strlen(data));
	// elog(LOG, "written: %d", n); // print number of bytes written to socket
	if (n < 0)
		return NO_DATA_SENT;

	// Read response
	response_received = false;
	timeouts_elapsed = 0;
	while (!response_received && timeouts_elapsed < 3) {
		n = read(sockfd, recvBuff, sizeof(recvBuff) - 1);
		timeouts_elapsed++;
		recvBuff[n] = 0;
		
		//elog(LOG, "received: %d", n); // print number of bytes received
		//elog(LOG, "N-1= %c", recvBuff[n -1]);
		//elog(LOG, "N-2= %c", recvBuff[n -2]);
		//elog(LOG, "N-3= %c", recvBuff[n -3]);
		
		if (recvBuff[n -2] == '+' || recvBuff[n - 2] == '-') 
		  response_received = true;
		
		/* Debug logging.  */
		/* if (n > 0)
			elog(LOG, "received: %s", recvBuff); */
	}
	
	/* Process Response */
	if (response_received) {
	  if(strcmp(recvBuff, "OK-") == 0) {
	    elog(LOG, "Relsys Server Responded OK-");
      //noop, but maybe do something in future
		}
	}
	
	return 0;  
}
