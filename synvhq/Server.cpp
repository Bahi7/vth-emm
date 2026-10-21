#include "PathOram.h"


static void deserializeData(std::unique_ptr<unsigned char* []>& data, const size_t& dataSize, const size_t& blockSize) {
	std::ifstream is("serialized_data.bin", std::ios::binary);
	
	if (!is) {
		std::cerr << "Cannot open file for reading." << std::endl;
		return;
	}

	cereal::BinaryInputArchive archive(is);

	// Deserialize each string
	for (size_t i = 0; i < dataSize; ++i) {
		// Now deserialize the string data
		for (size_t j = 0; j < blockSize; ++j) {
			archive(data[i][j]);
		}

	}
}

using asio::ip::tcp;

int main()
{

	
	asio::io_context io_context;
	size_t leafNode;

	// Listen for new connections
	tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), 3333));

	std::cout << "Server listening on port 3333...\n";
	tcp::socket socket(io_context);
	acceptor.accept(socket);
	std::cout << "Client connected: " << socket.remote_endpoint() << "\n";

	PathOram instanceOram;
	vector<size_t> attributes(5); 

	asio::read(socket, asio::buffer(attributes.data(), attributes.size() * sizeof(size_t)));

	instanceOram.setL(attributes[0]);
	instanceOram.setNoOfLeafs(attributes[1]);
	instanceOram.setSizeOfBlock(attributes[2]);
	instanceOram.setTreeSize(attributes[3]);
	instanceOram.setZ(attributes[4]);
	instanceOram.setPathLength(attributes[0] + 1);

	instanceOram.init_arrays();
	attributes.clear();

	size_t blockSize = instanceOram.getsizeOfBlock(); 
	

	std::unique_ptr<unsigned char*[]> data(new unsigned char*[instanceOram.getZ() * instanceOram.getTreeSize()]);
	for (size_t i = 0; i < instanceOram.getZ() * instanceOram.getTreeSize(); ++i)
	{
		data[i] = new unsigned char[blockSize];
	}
	deserializeData(data, instanceOram.getZ() * instanceOram.getTreeSize(), instanceOram.getsizeOfBlock());

	instanceOram.setTree(data);

	size_t n = instanceOram.response_size();
	size_t response_stash_size = n * blockSize;

	// Allocate the contiguous block
	unsigned char* dataBlock = new unsigned char[response_stash_size];

	// Create an array of pointers for easy access
	std::unique_ptr<unsigned char* []> encrypted_response_stash = std::make_unique<unsigned char* []>(n);

	// Point each element in 'data' to the correct location in 'dataBlock'
	for (size_t i = 0; i < n; ++i) {
		encrypted_response_stash[i] = &dataBlock[i * blockSize];
	}

	cout << response_stash_size / 1024 << " KB received !\n";
	
	asio::write(socket, asio::buffer("ok"));
	
	while (true) {
		
		asio::read(socket, asio::buffer(&leafNode, sizeof(size_t)));
		
		// Process the leaf node with ORAM instance
		instanceOram.access(leafNode, encrypted_response_stash);
		
		// Send the serialized path
		asio::write(socket, asio::buffer(dataBlock, response_stash_size));
		
		asio::read(socket, asio::buffer(dataBlock, response_stash_size));
		
		// Process the response with ORAM instance
		instanceOram.reWrite(encrypted_response_stash);
	

	}
};