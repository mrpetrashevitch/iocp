enum class packet_type : int
{
	reset,
	add,
	sub,
	get,
};

struct packet_header
{
	int size;
	packet_type type;
};