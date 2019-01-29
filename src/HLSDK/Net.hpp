#pragma once

#define NET_SUCCESS						( 0 )
#define NET_ERROR_TIMEOUT				( 1<<0 )
#define NET_ERROR_PROTO_UNSUPPORTED		( 1<<1 )
#define NET_ERROR_UNDEFINED				( 1<<2 )

typedef enum
{
	NA_UNUSED,
	NA_LOOPBACK,
	NA_BROADCAST,
	NA_IP,
	NA_IPX,
	NA_BROADCAST_IPX,
} netadrtype_t;

typedef struct netadr_s
{
	netadrtype_t	type;
	unsigned char	ip[4];
	unsigned char	ipx[10];
	unsigned short	port;
} netadr_t;

typedef struct net_adrlist_s
{
	struct net_adrlist_s	*next;
	netadr_t				remote_address;
} net_adrlist_t;

typedef struct net_response_s
{
	// NET_SUCCESS or an error code
	int			error;

	// Context ID
	int			context;
	// Type
	int			type;

	// Server that is responding to the request
	netadr_t	remote_address;

	// Response RTT ping time
	double		ping;
	// Key/Value pair string ( separated by backlash \ characters )
	// WARNING:  You must copy this buffer in the callback function, because it is freed
	//  by the engine right after the call!!!!
	// ALSO:  For NETAPI_REQUEST_SERVERLIST requests, this will be a pointer to a linked list of net_adrlist_t's
	void		*response;
} net_response_t;

typedef struct net_status_s
{
		// Connected to remote server?  1 == yes, 0 otherwise
	int			connected; 
	// Client's IP address
	netadr_t	local_address;
	// Address of remote server
	netadr_t	remote_address;
	// Packet Loss ( as a percentage )
	int			packet_loss;
	// Latency, in seconds ( multiply by 1000.0 to get milliseconds )
	double		latency;
	// Connection time, in seconds
	double		connection_time;
	// Rate setting ( for incoming data )
	double		rate;
} net_status_t;