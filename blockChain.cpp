#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include "blockChain.h"
#include <iostream>
#include<cstdlib>
using namespace std;
#pragma comment(lib, "pthreadVC2.lib")
namespace ShaCoin
{
	BlockChain::BlockChain()
	{
		pthread_mutex_init(&m_mutexTs, NULL);
		pthread_mutex_init(&m_mutexBlock, NULL);
		if (m_lst_block.size() == 0)
		{
			Block GenesisBlock;
			GenesisBlock.index = 1;
			GenesisBlock.timestamp = 20190513;
			GenesisBlock.lst_ts.clear();
			GenesisBlock.proof = 0;
			GenesisBlock.previous_hash = "0";
			InsertBlock(GenesisBlock);
		}
	}

	BlockChain::~BlockChain()
	{
		pthread_mutex_destroy(&m_mutexTs);
		pthread_mutex_destroy(&m_mutexBlock);
	}

	BlockChain *BlockChain::Instance()
	{
		static BlockChain bc;
		return &bc;
	}

	std::string BlockChain::GetJsonFromBlock(Block &block)
	{
		//使用boost::property_tree解析json
		boost::property_tree::ptree item;

		boost::property_tree::ptree lstts;
		{
			std::list<Transactions>::iterator it;
			for (it = block.lst_ts.begin(); it != block.lst_ts.end(); ++it)
			{
				boost::property_tree::ptree ts;
				//put()是ostream类的成员函数，功能是将一个字符写入文件
				ts.put("sender", it->sender);
				ts.put("recipient", it->recipient);
				ts.put("amount", it->amount);
				//std::pair主要的作用是将两个数据组合成一个数据???怎么操作的？
				lstts.push_back(make_pair("", ts));
			}
		}

		item.put("index", block.index);
		item.put("timestamp", block.timestamp);
		item.put_child("transactions", lstts);
		item.put("proof", block.proof);
		item.put("previous_hash", block.previous_hash);

		std::stringstream is;
		boost::property_tree::write_json(is, item);
		return is.str();
	}

	std::string BlockChain::GetJsonFromTransactions(Transactions &ts)
	{
		boost::property_tree::ptree item;

		item.put("sender", ts.sender);
		item.put("recipient", ts.recipient);
		item.put("amount", ts.amount);

		std::stringstream is;
		boost::property_tree::write_json(is, item);
		return is.str();
	}

	Block BlockChain::GetBlockFromJson(const std::string &json)//???
	{
		Block block;
		std::stringstream ss(json);
		boost::property_tree::ptree pt;
		boost::property_tree::ptree array;
		boost::property_tree::read_json(ss, pt);
		block.index = pt.get<int>("index");
		block.previous_hash = pt.get<std::string>("previous_hash");
		block.proof = pt.get<long int>("proof");
		block.timestamp = pt.get<time_t>("timestamp");
		array = pt.get_child("transactions");

		for (auto v : array)
		{
			Transactions ts;
			ts.sender = v.second.get<std::string>("sender");
			ts.recipient = v.second.get<std::string>("recipient");
			ts.amount = v.second.get<float>("amount");
			block.lst_ts.push_back(ts);
		}

		return block;
	}

	Transactions BlockChain::GetTransactionsFromJson(const std::string &json)
	{
		Transactions ts;
		std::stringstream ss(json);
		boost::property_tree::ptree pt;
		boost::property_tree::read_json(ss, pt);

		ts.sender = pt.get<std::string>("sender");
		ts.recipient = pt.get<std::string>("recipient");
		ts.amount = pt.get<float>("amount");

		return ts;
	}

	std::string BlockChain::GetJsonFromBlockList()
	{
		int i = 0;

		boost::property_tree::ptree item;

		boost::property_tree::ptree pblock;
		{
			std::list<Block>::iterator bit;

			pthread_mutex_lock(&m_mutexBlock);
			for (bit = m_lst_block.begin(); bit != m_lst_block.end(); ++bit)
			{
				boost::property_tree::ptree b;
				boost::property_tree::ptree pts;
				{
					std::list<Transactions>::iterator tit;
					for (tit = bit->lst_ts.begin(); tit != bit->lst_ts.end(); ++tit)
					{
						boost::property_tree::ptree t;
						t.put("sender", tit->sender);
						t.put("recipient", tit->recipient);
						t.put("amount", tit->amount);
						pts.push_back(make_pair("", t));
					}
				}

				b.put("index", bit->index);
				b.put("timestamp", bit->timestamp);
				b.put_child("transactions", pts);
				b.put("proof", bit->proof);
				b.put("previous_hash", bit->previous_hash);
				pblock.push_back(make_pair("", b));

				++i;
			}
			pthread_mutex_unlock(&m_mutexBlock);
		}

		item.put_child("chain", pblock);
		item.put("length", i);

		std::stringstream is;
		boost::property_tree::write_json(is, item);
		return is.str();
	}

	std::string BlockChain::GetJsonFromTransactionsList()
	{
		int i = 0;

		boost::property_tree::ptree item;

		boost::property_tree::ptree pts;
		{
			std::list<Transactions>::iterator bit;

			pthread_mutex_lock(&m_mutexTs);
			for (bit = m_lst_ts.begin(); bit != m_lst_ts.end(); ++bit)
			{
				boost::property_tree::ptree b;

				b.put("sender", bit->sender);
				b.put("recipient", bit->recipient);
				b.put("amount", bit->amount);

				pts.push_back(make_pair("", b));

				++i;
			}
			pthread_mutex_unlock(&m_mutexTs);
		}

		item.put_child("transactions", pts);
		item.put("length", i);

		std::stringstream is;
		boost::property_tree::write_json(is, item);
		return is.str();
	}

	std::list<Block> BlockChain::GetBlockListFromJson(const std::string &json)
	{
		std::list<Block> lst_block;
		std::stringstream ss(json);
		boost::property_tree::ptree pt;
		boost::property_tree::ptree barray;
		boost::property_tree::read_json(ss, pt);
		barray = pt.get_child("chain");

		for (auto bv : barray)
		{
			Block block;
			boost::property_tree::ptree tarray;

			block.index = bv.second.get<int>("index");
			block.previous_hash = bv.second.get<std::string>("previous_hash");
			block.proof = bv.second.get<long int>("proof");
			block.timestamp = bv.second.get<time_t>("timestamp");
			tarray = bv.second.get_child("transactions");

			for (auto tv : tarray)
			{
				Transactions ts;
				ts.sender = tv.second.get<std::string>("sender");
				ts.recipient = tv.second.get<std::string>("recipient");
				ts.amount = tv.second.get<float>("amount");
				block.lst_ts.push_back(ts);
			}

			lst_block.push_back(block);
		}

		return lst_block;
	}

	void BlockChain::GetTransactionsListFromJson(const std::string &json)
	{
		pthread_mutex_lock(&m_mutexTs);
		m_lst_ts.clear();
		pthread_mutex_unlock(&m_mutexTs);

		std::stringstream ss(json);
		boost::property_tree::ptree pt;
		boost::property_tree::ptree array;
		boost::property_tree::read_json(ss, pt);
		array = pt.get_child("transactions");

		for (auto v : array)
		{
			Transactions ts;
			ts.sender = v.second.get<std::string>("sender");
			ts.recipient = v.second.get<std::string>("recipient");
			ts.amount = v.second.get<float>("amount");

			pthread_mutex_lock(&m_mutexTs);
			m_lst_ts.push_back(ts);
			pthread_mutex_unlock(&m_mutexTs);
		}
	}
	//地址是把公钥用base64编码的结果，其实也可以把公钥hash得到
	std::string BlockChain::CreateNewAddress(const KeyPair &keyPair)
	{
		std::string hash = Cryptography::GetHash(keyPair.pubKey.key, keyPair.pubKey.len);
		return Cryptography::Base64Encode(hash.c_str(), hash.length());
	}

	Transactions BlockChain::CreateTransactions(const std::string &sender, const std::string &recipient, float amount)
	{
		Transactions ts;
		ts.sender = sender;
		ts.recipient = recipient;
		ts.amount = amount;
		return ts;
	}
	//矿工挖出proof后调用
	Block BlockChain::CreateBlock(int index, time_t timestamp, long int proof)
	{
		Block block;

		ShaCoin::Block last = GetLastBlock();
		std::string strLastBlock = GetJsonFromBlock(last);

		block.index = index;
		block.timestamp = timestamp;
		block.proof = proof;
	//c_str()函数返回一个指向正规C字符串的指针常量, 内容与本string串相同. 
		//对上一个区块哈希
		block.previous_hash = Cryptography::GetHash(strLastBlock.c_str(), strLastBlock.length());

		pthread_mutex_lock(&m_mutexTs);//锁住交易池线程？
		block.lst_ts = m_lst_ts;//区块链交易池直接复制到区块上

		m_lst_ts.clear();//清空交易池
		pthread_mutex_unlock(&m_mutexTs);

		return block;
	}

	int BlockChain::WorkloadProof(int last_proof)//工作量证明：将上一个区块的数据作为工作量证明的输入
	{
		std::string strHash;
		std::string strTemp;
		//类比比特币：使用字符串“blockchain”，在字符串后面加上nonce的整数值串。
		//工作量要求是：在“blockchain” + nonce进行SHA256哈希运算，如果得到哈希结果
		//（以十六进制表示）是以若干个0开头，则验证通过。
		int randomNum = rand();
		int proof = last_proof + randomNum ;

		std::string str = "Hello Shacoin!";
		int count = 1;
		while (true)
		{
			
			strTemp = str + std::to_string(proof);
			strHash = Cryptography::GetHash(strTemp.c_str(), strTemp.length());//对什么做哈希?
			if (strHash.back() == '0') {//返回strHash的最后一个字符
				cout << "挖到了" << endl;
				return proof;
			}
			else {
				++proof;
				cout << "第" << count++ << "次挖矿" << endl;
			}
				
		}
	}

	bool BlockChain::WorkloadVerification(int proof)
	{
		std::string str = "Hello Shacoin!" + std::to_string(proof);
		std::string strHash = Cryptography::GetHash(str.c_str(), str.length());
		return (strHash.back() == '0');
	}
	//矿工把自己的地址作为参数输入，coinbase里的小费存入矿工地址
	std::string BlockChain::Mining(const std::string &addr)
	{
		
		Block last = GetLastBlock();
		int proof = WorkloadProof(last.proof);
		Transactions ts = CreateTransactions("sender", addr, 10);//coinbase交易
		InsertTransactions(ts);//coinbase交易放入交易池
		Block block = CreateBlock(last.index + 1, time(NULL), proof);
		//区块打包，写在了CreatBlock里，默认打包所有交易池的交易
		return GetJsonFromBlock(block);
	}

	int BlockChain::CheckBalances(const std::string &addr)
	{
		cout << "function balance" << endl;
		int balan = 0;

		std::list<Block>::iterator bit;
		std::list<Transactions>::iterator tit;

		pthread_mutex_lock(&m_mutexBlock);
		for (bit = m_lst_block.begin(); bit != m_lst_block.end(); ++bit)
		{
			for (tit = bit->lst_ts.begin(); tit != bit->lst_ts.end(); ++tit)
			{
				if (tit->recipient == addr)
					balan += tit->amount;
				else if (tit->sender == addr)
					balan -= tit->amount;
			}
		}
		pthread_mutex_unlock(&m_mutexBlock);

		return balan;
	}
	//这个函数还没有被调用过
	//删除交易池里已经被区块打包过的交易
	void BlockChain::DeleteDuplicateTransactions(const Block &block)
	{
		std::list<Transactions>::iterator selfIt;
		//这个变量没什么用
		//std::list<Transactions>::const_iterator otherIt;

		pthread_mutex_lock(&m_mutexTs);
		for (selfIt = m_lst_ts.begin(); selfIt != m_lst_ts.end();)
		{	//如果区块里的交易在交易池里，就删掉
			if (block.lst_ts.end() != std::find(block.lst_ts.begin(), block.lst_ts.end(), *selfIt))
			{
				selfIt = m_lst_ts.erase(selfIt);
			}
			else
			{
				++selfIt;
			}
		}
		pthread_mutex_unlock(&m_mutexTs);
	}
	//没被调用，这不是区块链逻辑
	//如果顺序出错需要调整
	void BlockChain::MergeBlockChain(const std::string &json)
	{
		std::list<Block> lst_block = GetBlockListFromJson(json);
		lst_block.pop_front();
		//如果收到广播的区块数大于区块池的容量
		if (lst_block.size() > m_lst_block.size())
		{
			std::list<Block>::iterator it;

			pthread_mutex_lock(&m_mutexBlock);

			for (it = m_lst_block.begin(); it != m_lst_block.end(); ++it)
			{
				Block block = lst_block.back();
				if (it->proof <= block.proof)
					continue;
				std::string strJson = GetJsonFromBlock(block);
				std::string strHash = Cryptography::GetHash(strJson.c_str(), strJson.length());

				it->index = block.index + 1;
				it->previous_hash = strHash;
				lst_block.push_back(*it);
			}

			m_lst_block = lst_block;

			pthread_mutex_unlock(&m_mutexBlock);
		}
		else
		{
			std::list<Block>::iterator it;
			for (it = lst_block.begin(); it != lst_block.end(); ++it)
			{
				pthread_mutex_lock(&m_mutexBlock);

				Block block = m_lst_block.back();
				if (it->proof <= block.proof)
				{
					pthread_mutex_unlock(&m_mutexBlock);
					continue;
				}
				std::string strJson = GetJsonFromBlock(block);
				std::string strHash = Cryptography::GetHash(strJson.c_str(), strJson.length());

				it->index = block.index + 1;
				it->previous_hash = strHash;
				m_lst_block.push_back(*it);

				pthread_mutex_unlock(&m_mutexBlock);
			}
		}
	}
}
