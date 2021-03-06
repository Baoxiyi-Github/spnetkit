/*
 * Copyright 2008 Stephen Liu
 * For license terms, see the file COPYING along with this library.
 */

#ifndef __spnksocket_hpp__
#define __spnksocket_hpp__

#include "spnkporting.hpp"

typedef struct tagSP_NKSocketImpl SP_NKSocketImpl_t;

class SP_NKSocket {
public:
	enum {
		DEFAULT_SOCKET_TIMEOUT  = 600,
		DEFAULT_CONNECT_TIMEOUT = 60
	};

	/**
	 * return > 0 : how many events
	 * return 0 : timeout,
	 * return -1 : error, and errno is set appropriately
	 */
	static int poll( int fd, int events, int * revents, int timeoutSeconds );

	static int poll( int fd, int events, int * revents, const struct timeval * timeout );

	static void setLogSocketDefault( int logSocket );

	static int setNonblocking( int fd );

	static int tcpListen( const char * ip, int port, int * fd, int blocking = 1 );

protected:
	static int mLogSocketDefault;

public:

	virtual ~SP_NKSocket();

	void setSocketTimeout( int socketTimeoutSeconds );

	void setLogSocket( int logSocket );

	// get the underlying socket file descriptor
	int getSocketFd();

	int detachSocketFd();

	const char * getPeerHost();

	int getPeerPort();

	time_t getLastActiveTime();

	/**
	 * @return -1 : error, and errno is set appropriately
	 * @return > 0 : recv bytes, include trailing '\n' or '\r\n'
	 * @return 0 : connection close by peer
	 * @param len : readline does not write more than len bytes
	 *       (including the trailing '\0')
	 * @param buffer : excluding the trailing '\n' or '\r\n'
	 */
	int readline( char * buffer, size_t len );

	/**
	 * @return -1 : error, and errno is set appropriately
	 * @return > 0 : recv bytes
	 * @return 0 : connection close by peer
	 */
	int read( void * buffer, size_t len );

	/**
	 * @return -1 : error, and errno is set appropriately
	 * @return > 0 : recv bytes
	 * @return 0 : connection close by peer
	 */
	int readn( void * buffer, size_t len );

	/**
	 * push buffer back to stream, where it is available for subsequent read
	 * operations. Pushed back characters will be returned in reverse order.
	 *
	 * @return -1 : too many data, change nothing
	 * @return >= 0 : unread bytes
	 */
	int unread( void * buffer, size_t len );

	/**
	 * @return -1 : error, and errno is set appropriately
	 * @return > 0 : send bytes
	 */
	int printf( const char * format, ... );

	/**
	 * @return -1 : error, and errno is set appropriately
	 * @return > 0 : send bytes
	 */
	int writen( const void * buffer, size_t len );

	/**
	 * @return -1 : error, and errno is set appropriately
	 * @return > 0 : recv bytes
	 * @return 0 : connection close by peer
	 */
	int probe( void * buffer, size_t len );

	// return internal pending buffer and length
	size_t peek( char ** const buffer = NULL );

	int close();

protected:

	SP_NKSocket();

	void init( int socketFd, int toBeOwner );

	virtual int realRecv( int fd, void * buffer, size_t len ) = 0;
	virtual int realSend( int fd, const void * buffer, size_t len ) = 0;

private:

	SP_NKSocketImpl_t * mImpl;
};

class SP_NKTcpSocket : public SP_NKSocket {
public:
	static int openSocket( const char * ip, int port,
		const struct timeval * connectTimeout, const char * bindAddr );

	static int openSocket( const char * path,
		const struct timeval * connectTimeout );

public:
	SP_NKTcpSocket( int socketFd );

	SP_NKTcpSocket( const char * ip, int port,
			int connectTimeoutSeconds = 0, const char * bindAddr = 0 );

	SP_NKTcpSocket( const char * ip, int port,
			const struct timeval * connectTimeout, const char * bindAddr = 0 );

	SP_NKTcpSocket( const char * path, const struct timeval * connectTimeout = 0 );

	virtual ~SP_NKTcpSocket();

private:

	static int connectNonblock( int sockFd, struct sockaddr * addr,
		socklen_t addrLen, const struct timeval * connectTimeout );

	virtual int realRecv( int fd, void * buffer, size_t len );
	virtual int realSend( int fd, const void * buffer, size_t len );
};

class SP_NKUdpSocket : public SP_NKSocket {
public:
	SP_NKUdpSocket( int socketFd, struct sockaddr * addr, socklen_t len );

	SP_NKUdpSocket( const char * ip, int port );

	virtual ~SP_NKUdpSocket();

private:
	struct sockaddr * mAddr;
	socklen_t mLen;

	virtual int realRecv( int fd, void * buffer, size_t len );
	virtual int realSend( int fd, const void * buffer, size_t len );
};

#endif

