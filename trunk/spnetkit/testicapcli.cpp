/*
 * Copyright 2009 Stephen Liu
 * For license terms, see the file COPYING along with this library.
 */

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "spnkporting.hpp"

#include "spnkhttpmsg.hpp"
#include "spnkicapcli.hpp"
#include "spnksocket.hpp"

#include "spnklog.hpp"
#include "spnkfile.hpp"

#include "spnkgetopt.h"

void showUsage( const char * program )
{
	printf( "\n\n%s [-h host] [-p port] [-r RESPMOD|OPTIONS] [-u URI] [-f file] [-o] [-v]\n", program );

	printf( "\t-h icap host\n" );
	printf( "\t-p icap port\n" );
	printf( "\t-r icap method, only support RESPMOD/OPTIONS\n" );
	printf( "\t-u icap URI\n" );
	printf( "\t-f the file for RESPMOD body, only need for RESPMOD\n" );
	printf( "\t-o log socket io\n" );
	printf( "\t-v show this usage\n" );
	printf( "\n\n" );

	exit( 0 );
}

int main( int argc, char * argv[] )
{
#ifndef WIN32
	assert ( sigset ( SIGPIPE, SIG_IGN ) != SIG_ERR ) ;
#endif

	SP_NKLog::init4test( "testicapcli" );
	SP_NKLog::setLogLevel( LOG_DEBUG );

	char * host = NULL, * port = NULL, * method = NULL, * uri = NULL, * file = NULL;

	extern char *optarg ;
	int c ;

	while( ( c = getopt( argc, argv, "h:p:r:u:f:ov" ) ) != EOF ) {
		switch ( c ) {
			case 'h' : host = optarg; break;
			case 'p' : port = optarg; break;
			case 'r' : method = optarg; break;
			case 'u' : uri = optarg; break;
			case 'f' : file = optarg; break;
			case 'o' : SP_NKSocket::setLogSocketDefault( 1 ); break;
			case 'v' :
			default: showUsage( argv[ 0 ] ); break;
		}
	}

	if( NULL == host || NULL == host ) {
		printf( "Please specify host and port!\n" );
		showUsage( argv[ 0 ] );
	}

	if( NULL == method || NULL == uri ) {
		printf( "Please specify URI and method!\n" );
		showUsage( argv[ 0 ] );
	}

	if( 0 == strcasecmp( method, "RESPMOD" ) && NULL == file ) {
		printf( "Please specify the file for RESPMOD body!\n" );
		showUsage( argv[ 0 ] );
	}

	if( 0 != spnk_initsock() ) assert( 0 );

	SP_NKHttpRequest request;
	request.setURI( uri );
	request.setMethod( method );
	request.setVersion( "ICAP/1.0" );
	request.addHeader( "Connection", "Keep-Alive" );
	request.addHeader( "Host", "127.0.0.1" );

	if( 0 == strcasecmp( method, "RESPMOD" ) ) {
		SP_NKFileReader fileReader;
		if( 0 != fileReader.read( file ) ) {
			printf( "Cannot read %s", file );
			exit( 0 );
		} else {
			request.appendContent( fileReader.getBuffer(), fileReader.getSize() );
		}
	}

	SP_NKTcpSocket socket( host, atoi( port ) );

	SP_NKHttpResponse response;

	int ret = -1;

	if( request.isMethod( "RESPMOD" ) ) {
		ret = SP_NKIcapProtocol::respMod( &socket, uri, (char*)request.getContent(),
				request.getContentLength(), &response );
	} else if( request.isMethod( "OPTIONS" ) ) {
		ret = SP_NKIcapProtocol::options( &socket, uri, &response );
	} else {
		printf( "unsupport method %s\n", request.getMethod() );
	}

	if( 0 == ret ) {
		printf( "response:\n" );

		printf( "%s %d %s\n", response.getVersion(), response.getStatusCode(),
				response.getReasonPhrase() );

		printf( "\r\n" );

		printf( "%d headers\n", response.getHeaderCount() );
		for( int i = 0; i < response.getHeaderCount(); i++ ) {
			const char * name = response.getHeaderName( i );
			const char * val = response.getHeaderValue( i );
			printf( "%s: %s\r\n", name, val );
		}

		printf( "\r\n" );

		printf( "%d bytes body\n", response.getContentLength() );
		if( NULL != response.getContent() && response.getContentLength() > 0 ) {
			printf( "%s\n", (char*)response.getContent() );
		}
	} else {
		printf( "http request fail\n" );
	}

	return 0;
}

