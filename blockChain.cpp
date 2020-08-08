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
		//ʹ��boost::property_tree����json
		boost::property_tree::ptree item;

		boost::property_tree::ptree lstts;
		{
			std::list<Transactions>::iterator it;
			for (it = block.lst_ts.begin(); it != block.lst_ts.end(); ++it)
			{
				boost::property_tree::ptree ts;
				//put()��ostream��ĳ�Ա�����������ǽ�һ���ַ�д���ļ�
				ts.put("sender", it->sender);
				ts.put("recipient", it->recipient);
				ts.put("amount", it->amount);
				//std::pair��Ҫ�������ǽ�����������ϳ�һ������???��ô�����ģ�
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
	//��ַ�ǰѹ�Կ��base64����Ľ������ʵҲ���԰ѹ�Կhash�õ�
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
	//���ڳ�proof�����
	Block BlockChain::CreateBlock(int index, time_t timestamp, long int proof)
	{
		Block block;

		ShaCoin::Block last = GetLastBlock();
		std::string strLastBlock = GetJsonFromBlock(last);

		block.index = index;
		block.timestamp = timestamp;
		block.proof = proof;
	//c_str()��������һ��ָ������C�ַ�����ָ�볣��, �����뱾string����ͬ. 
		//����һ�������ϣ
		block.previous_hash = Cryptography::GetHash(strLastBlock.c_str(), strLastBlock.length());

		pthread_mutex_lock(&m_mutexTs);//��ס���׳��̣߳�
		block.lst_ts = m_lst_ts;//���������׳�ֱ�Ӹ��Ƶ�������

		m_lst_ts.clear();//��ս��׳�
		pthread_mutex_unlock(&m_mutexTs);

		return block;
	}

	int BlockChain::WorkloadProof(int last_proof)//������֤��������һ�������������Ϊ������֤��������
	{
		std::string strHash;
		std::string strTemp;
		//��ȱ��رң�ʹ���ַ�����blockchain�������ַ����������nonce������ֵ����
		//������Ҫ���ǣ��ڡ�blockchain�� + nonce����SHA256��ϣ���㣬����õ���ϣ���
		//����ʮ�����Ʊ�ʾ���������ɸ�0��ͷ������֤ͨ����
		int randomNum = rand();
		int proof = last_proof + randomNum ;

		std::string str = "Hello Shacoin!";
		int count = 1;
		while (true)
		{
			
			strTemp = str + std::to_string(proof);
			strHash = Cryptography::GetHash(strTemp.c_str(), strTemp.length());//��ʲô����ϣ?
			if (strHash.back() == '0') {//����strHash�����һ���ַ�
				cout << "�ڵ���" << endl;
				return proof;
			}
			else {
				++proof;
				cout << "��" << count++ << "���ڿ�" << endl;
			}
				
		}
	}

	bool BlockChain::WorkloadVerification(int proof)
	{
		std::string str = "Hello Shacoin!" + std::to_string(proof);
		std::string strHash = Cryptography::GetHash(str.c_str(), str.length());
		return (strHash.back() == '0');
	}
	//�󹤰��Լ��ĵ�ַ��Ϊ�������룬coinbase���С�Ѵ���󹤵�ַ
	std::string BlockChain::Mining(const std::string &addr)
	{
		
		Block last = GetLastBlock();
		int proof = WorkloadProof(last.proof);
		Transactions ts = CreateTransactions("sender", addr, 10);//coinbase����
		InsertTransactions(ts);//coinbase���׷��뽻�׳�
		Block block = CreateBlock(last.index + 1, time(NULL), proof);
		//��������д����CreatBlock�Ĭ�ϴ�����н��׳صĽ���
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
	//���������û�б����ù�
	//ɾ�����׳����Ѿ������������Ľ���
	void BlockChain::DeleteDuplicateTransactions(const Block &block)
	{
		std::list<Transactions>::iterator selfIt;
		//�������ûʲô��
		//std::list<Transactions>::const_iterator otherIt;

		pthread_mutex_lock(&m_mutexTs);
		for (selfIt = m_lst_ts.begin(); selfIt != m_lst_ts.end();)
		{	//���������Ľ����ڽ��׳����ɾ��
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
	//û�����ã��ⲻ���������߼�
	//���˳�������Ҫ����
	void BlockChain::MergeBlockChain(const std::string &json)
	{
		std::list<Block> lst_block = GetBlockListFromJson(json);
		lst_block.pop_front();
		//����յ��㲥����������������ص�����
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
