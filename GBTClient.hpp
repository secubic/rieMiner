// (c) 2018-2020 Pttn (https://github.com/Pttn/rieMiner)

#ifndef HEADER_GBTClient_hpp
#define HEADER_GBTClient_hpp

#include "Client.hpp"
#include "tools.hpp"

// Stores the GetBlockTemplate data got from the RPC call
struct GetBlockTemplateData {
	BlockHeader bh;
	std::string transactions, default_witness_commitment; // In hex format
	std::vector<std::array<uint8_t, 32>> txHashes;
	uint64_t coinbasevalue;
	uint32_t height;
	std::vector<uint8_t> coinbase, // Store Coinbase Transaction here
	                     scriptPubKey; // Calculated from custom payout address for Coinbase Transaction
	std::vector<std::string> rules; // From GetBlockTemplate response
	int32_t powVersion;
	std::vector<std::vector<uint64_t>> acceptedConstellationOffsets;
	
	GetBlockTemplateData() : coinbasevalue(0), height(0) {}
	void coinBaseGen(const AddressFormat&, const std::string&, uint16_t);
	std::array<uint8_t, 32> coinBaseHash() const {
		if (default_witness_commitment.size() > 0) { // For SegWit, hash to get txid rather than just hash the whole Coinbase
			std::vector<uint8_t> coinbase2;
			for (uint32_t i(0) ; i < 4 ; i++) coinbase2.push_back(coinbase[i]); // nVersion
			for (uint32_t i(6) ; i < coinbase.size() - 38 ; i++) coinbase2.push_back(coinbase[i]); // txins . txouts
			for (uint32_t i(coinbase.size() - 4) ; i < coinbase.size() ; i++) coinbase2.push_back(coinbase[i]); // nLockTime
			return v8ToA8(sha256sha256(coinbase2.data(), coinbase2.size()));
		}
		else return v8ToA8(sha256sha256(coinbase.data(), coinbase.size()));
	}
	void merkleRootGen() {
		const std::array<uint8_t, 32> mr(calculateMerkleRoot(txHashes));
		memcpy(bh.merkleRoot, mr.data(), 32);
	}
	bool isActive(const std::string &rule) const {
		for (uint32_t i(0) ; i < rules.size() ; i++) {
			if (rules[i] == rule) return true;
		}
		return false;
	}
};

// Client for the GetBlockTemplate protocol (solo mining)
class GBTClient : public Client {
	GetBlockTemplateData _gbtd;
	
	std::string _getUserPass() const; // Returns "username:password", for sendRPCCall(...)
	std::string _getHostPort() const; // Returns "http://host:port/", for sendRPCCall(...)
	bool _getWork(); // Via getblocktemplate
	
public:
	using Client::Client;
	bool connect();
	void updateMinerParameters(MinerParameters&);
	json_t* sendRPCCall(const std::string&) const; // Send a RPC call to the server and returns the response
	void sendWork(const WorkData&) const; // Via submitblock
	WorkData workData();
	virtual uint32_t currentHeight() const {return _gbtd.height;}
	virtual uint32_t currentDifficulty() const {return decodeCompact(_gbtd.bh.bits);}
};

#endif
