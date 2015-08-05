#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <signal.h>
#include <string.h>
#include <iostream>

#include "../spnkporting.hpp"
#include "../spnkhttpcli.hpp"
#include "../spnkhttpmsg.hpp"
#include "../spnksocket.hpp"
#include "../spnklog.hpp"
#include "../spnkfile.hpp"
#include "../spnkgetopt.h"

using namespace std;
void showUsage(const char *program)
{
    cout << program << "-h [host] -p [post] -r [method] -u [uri] -f [file] [-o] [-v]" << endl;
    cout << "\t -h http post " << endl;
    cout << "\t -p http post " << endl;
    cout << "\t -r http method, only support POST/GET/HEAD" << endl;
    cout << "\t -u http URI" << endl;
    cout << "\t -f the file for POST body, only need for POST" << endl;
    cout << "\t -o log socket io " << endl;
    cout << "\t -v show this page " << endl;
}

int main(int argc, char *argv[])
{
    SP_NKLog::init4test("mytest");
    SP_NKLog::setLogLevel(LOG_DEBUG);

    char *uri = NULL, *port = NULL, *host = NULL, *method = NULL, *file = NULL;

    extern char *optarg;
    int c;

    while ( ( c = getopt(argc, argv, "h:p:r:u:f:vo" ) ) != EOF ) {
        switch ( c ){
            case 'h': host = optarg; break;
            case 'p': port = optarg; break;
            case 'r': method = optarg; break;
            case 'u': uri = optarg; break;
            case 'f': file = optarg; break;
            case 'o': SP_NKSocket::setLogSocketDefault(1); break;
            case 'v':
            default : showUsage(argv[0]); break;
        }
    }

    
    if ( NULL == host || NULL == port ) {
        cout << "Please specifly host and post" << endl;
        showUsage(argv[0]);
    }

    if ( NULL == method || NULL == uri ) {
        cout << "Please specify method and uri" << endl;
        showUsage(argv[0]);
    }

    if ( 0 == strcasecmp( method, "POST" ) && NULL == file ) {
        cout << "Please specify the file for POST body " << endl;
        showUsage(argv[0]);
    }

    if(0 != spnk_initsock() ) {
        assert(0);
    }

    SP_NKHttpRequest request;

    request.setURI( uri );
    request.setMethod( method );
    request.setVersion( "Http/1.1" );
    request.addHeader( "connection", "keep-Alive" );
    request.addHeader( "Host", "127.0.0.1" );

    if ( 0 == strcasecmp( method , "POST" ) ) {
        SP_NKFileReader fileReader;
        if ( 0 != fileReader.read( file ) ) {
            cout << "cannot read " << file << endl;
            exit(0);
        }else{
            request.appendContent(fileReader.getBuffer(), fileReader.getSize());
        }
    }

    SP_NKTcpSocket socket(host, atoi(port));
        
    SP_NKHttpResponse response;

    int ret = -1;

    if(request.isMethod( "GET" ) ) {
        ret = SP_NKHttpProtocol::get ( &socket, &request, &response );
    } else if (request.isMethod( "POST" ) ) {
        ret = SP_NKHttpProtocol::post ( &socket, &request, &response );
    } else if (request.isMethod( "HEAD" ) ) {
        ret = SP_NKHttpProtocol::head ( &socket, &request, &response );
    } else {
        cout << "unsupport Method " << request.getMethod() << endl;
    }

    if ( 0 == ret ) {
        cout << "response: " << endl;
        cout << response.getVersion() << response.getStatusCode() << response.getReasonPhrase() << endl;

        cout << response.getHeaderCount() << endl;

        for ( int i = 0; i < response.getHeaderCount(); i++ ) {
            const char *name = response.getHeaderName(i);
            const char *value = response.getHeaderValue(i);

            cout << name << ": " << value << endl;
        }
    
        cout << response.getContentLength() << endl;

        if ( NULL != response.getContent() && response.getContentLength() ) {
            cout << (char *)response.getContent() << endl;
        }
    } else {
        cout << " Http response failed " << endl;
    }
    return 0;
}
