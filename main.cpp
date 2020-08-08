#include <iostream>

//#include <unistd.h>

//#include "p2pServer.hpp"
//#include "p2pNode.hpp"
#include "blockChain.h"
#include "cryptography.h"

/*
{
index:1
hash:gydsgyuagv
	{
	Tx:
	send:
	rec:
	}

}
std::list<Transactions> m_lst_ts;
		std::list<Block> m_lst_block;
		pthread_mutex_t m_mutexTs;
		pthread_mutex_t m_mutexBlock;

*/
using namespace std;

int main(int argc, char **argv)
{
	/*
	if (argc < 2)
	{
		std::cout << "argc error!" << std::endl;
		return 0;
	}
	*/

	std::string strInputCmd;

	//ShaCoin::P2PNode *p2pNode = ShaCoin::P2PNode::Instance(argv[1]);
	//p2pNode->Listen();

	//创建区块链、写入创世区块
	ShaCoin::BlockChain *blockChain = ShaCoin::BlockChain::Instance();
	//ShaCoin::Block b1 = blockChain->CreateBlock(02, 6.3, 293471924);
	//std::cout << blockChain->GetJsonFromBlockList() << std::endl;
	//blockChain->InsertBlock(b1);
	std::cout << blockChain->GetJsonFromBlockList() << std::endl;

	while (1)
	{
		std::cout << "Please input command:" << std::endl;
		getline(std::cin, strInputCmd);
		std::vector<std::string> vec_str = ShaCoin::Cryptography::StringSplit(strInputCmd, ' ');//分割字符串

		if (vec_str.size() < 1)
			continue;

		if (ShaCoin::Cryptography::CompareNoCase(vec_str[0], "addr"))//生成账户
		{
			ShaCoin::KeyPair keyPair;
			ShaCoin::Cryptography::Createkey(keyPair);
			std::string addr = blockChain->CreateNewAddress(keyPair);

			std::cout << "Public key is " << ShaCoin::Cryptography::Base64Encode(keyPair.pubKey.key, keyPair.pubKey.len) << std::endl;//base64encode转为字节码格式再加密
			std::cout << "Private key is " << ShaCoin::Cryptography::Base64Encode(keyPair.priKey.key, keyPair.priKey.len) << std::endl;
			std::cout << "Address is " << addr << std::endl;
			continue;
		}
		//输入格式：ts 地址1 地址2 交易额float 公钥 私钥
		else if (ShaCoin::Cryptography::CompareNoCase(vec_str[0], "ts"))//
		{
			if (vec_str.size() < 6)//
				continue;

			ShaCoin::KeyPair kp;
			//作用是将某一块内存中的内容全部设置为指定的值， 函数通常为新申请的内存做初始化工作
			memset(&kp, 0, sizeof(kp));//将已开辟内存空间 kp 的kp个字节的值设为值0
			ShaCoin::Cryptography::Base64Decode(vec_str[4], kp.pubKey.key, sizeof(kp.pubKey.key), &kp.pubKey.len);
			ShaCoin::Cryptography::Base64Decode(vec_str[5], kp.priKey.key, sizeof(kp.priKey.key), &kp.priKey.len);

			ShaCoin::Transactions ts = blockChain->CreateTransactions(vec_str[1], vec_str[2], stof(vec_str[3]));
			std::string strTsJson = blockChain->GetJsonFromTransactions(ts);
			std::string strHash = ShaCoin::Cryptography::GetHash(strTsJson.c_str(), strTsJson.length());
				
//			ShaCoin::BroadcastMessage tm;
//			memset(&tm, 0, sizeof(tm));
//			tm.pubkey = kp.pubKey;
//			strncpy(tm.json, strTsJson.c_str(), strTsJson.length());
//			if (ShaCoin::Cryptography::Signature(kp.priKey, strHash.c_str(), strHash.length(), tm.sign, sizeof(tm.sign), &tm.signlen))
//				p2pNode->Broadcast(ShaCoin::p2p_transaction, tm);

			continue;
		}
		else if (ShaCoin::Cryptography::CompareNoCase(vec_str[0], "Mining"))
		{
			if (vec_str.size() < 2)
				continue;

			int count;
			if (vec_str.size() >= 3)
				count = stoi(vec_str[2]);//将挖矿次数字符串转化为十进制
			else
				count = 1;//若不输入挖矿次数则默认挖1个

			if (count < 1)
				count = 1;

			while (count)
			{
				std::string strMiningJson = blockChain->Mining(vec_str[1]);

				//ShaCoin::BroadcastMessage bmMining;
				//memset(&bmMining, 0, sizeof(bmMining));
				//strncpy(bmMining.json, strMiningJson.c_str(), strMiningJson.length());
				//p2pNode->Broadcast(ShaCoin::p2p_bookkeeping, bmMining);

				--count;
			}

			continue;
		}
		//上链 要改，应该是共识
		else if (ShaCoin::Cryptography::CompareNoCase(vec_str[0], "Merge"))
		{
//			p2pNode->MergeChain();
			continue;
		}
		//遍历区块和交易查询余额
		else if (ShaCoin::Cryptography::CompareNoCase(vec_str[0], "Balances"))
		{
			if (vec_str.size() < 2)
				continue;

			std::cout << blockChain->CheckBalances(vec_str[1]) << std::endl;

			continue;
		}

		else if (ShaCoin::Cryptography::CompareNoCase(vec_str[0], "show"))
		{
			std::cout << blockChain->GetJsonFromBlockList() << std::endl;

			continue;
		}
		else if (ShaCoin::Cryptography::CompareNoCase(vec_str[0], "help"))
		{
			std::cout << "<addr> create a new address and key pair." << std::endl;
			std::cout << "<ts> initiate a new benefit,the parameters are in order:send address, recipient address, amount." << std::endl;
			std::cout << "<mining> mining.the parameters are in order:mining address,number of mining times" << std::endl;
			std::cout << "<merge> blockchain merge." << std::endl;
			std::cout << "<balances> get the balance.the parameters are in order:address" << std::endl;
			std::cout << "<show> display blockchain in json format." << std::endl;
			std::cout << "<quit> quit." << std::endl;
			std::cout << "<help> show this message." << std::endl;
			continue;
		}
		else if (ShaCoin::Cryptography::CompareNoCase(vec_str[0], "quit"))
		{
			break;
		}
	}

	return 0;
}
#include <iostream>

//#include <unistd.h>

//#include "p2pServer.hpp"
//#include "p2pNode.hpp"
//#include "blockChain.h"
//#include "cryptography.h"
//using namespace ShaCoin;
//using namespace std;
//void main() {
//	KeyPair keyPair;
//	Cryptography::Createkey(keyPair);
//	cout << "PK:" << keyPair.pubKey.len<< endl;
//	for (int i = 0; i < keyPair.pubKey.len; i++) {
//		if (!(i % 8))
//			cout << endl;
//		if (i == keyPair.pubKey.len - 1)
//			printf("%02X", keyPair.pubKey.key[i]);
//		else
//			printf("%02X", keyPair.pubKey.key[i]);
//	}
//	cout << endl;
//	cout << "SK+PK等参数:" << keyPair.priKey.len << endl;
//	for (int i = 0; i < keyPair.priKey.len; i++) {
//		if (!(i % 8))
//			cout << endl;
//		if (i == 53)
//		{
//			cout << endl;
//			cout << endl;
//		}
//		
//		if (i == keyPair.priKey.len - 1)
//			printf("%02X", keyPair.priKey.key[i]);
//		else
//			printf("%02X", keyPair.priKey.key[i]);
//	}
//


	//std::cout << "Public key is " << ShaCoin::Cryptography::Base64Encode(keyPair.pubKey.key, keyPair.pubKey.len) << std::endl;//base64encode转为字节码格式再加密
	//std::cout << "Private key is " << ShaCoin::Cryptography::Base64Encode(keyPair.priKey.key, keyPair.priKey.len) << std::endl;

	//std::cout << "pk_len " << (ShaCoin::Cryptography::Base64Encode(keyPair.pubKey.key, keyPair.pubKey.len)).length() << endl;
	//std::cout << "sk_len " << ShaCoin::Cryptography::Base64Encode(keyPair.priKey.key, keyPair.priKey.len).length() << endl;


	
	
	
	