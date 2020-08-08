#ifndef __BLOCK_H
#define __BLOCK_H

/*

block = {
'index': 1,
'timestamp': 1506057125.900785,
'transactions': [
{
'sender': "8527147fe1f5426f9dd545de4b27ee00",
'recipient': "a77f5cdfa2934df3954a5c7c7da5df1f",
'amount': 5,
}
],
'proof': 324984774000,
'previous_hash': "2cf24dba5fb0a30e26e83b2ac5b9e29e1b161e5c1fa7425e73043362938b9824"
}

*/
#define HAVE_STRUCT_TIMESPEC////此处改动
#include <list>
#include <string>
#include <ctime>
#include <algorithm>
#include<pthread.h>////////////////有改动
#include "cryptography.h"
//clear
namespace ShaCoin
{
	typedef struct __transactions//交易类
	{
		std::string sender;
		std::string recipient;
		float amount;

		bool operator == (const struct __transactions & value) const//重载==操作符，用于比较两个区块是否相同
		{
			return
				this->sender == value.sender &&
				this->recipient == value.recipient &&
				this->amount == value.amount;
		}
	}Transactions;

	typedef struct __block//区块类
	{
		int index;
		/*time_t类型数据用来保存从1970年1月1日0时0分0秒到现在时刻的秒数！用time()这个函数获取！*/
		time_t timestamp;
		std::list<Transactions> lst_ts;
		long int proof;//上一个区块的proof+一个随机数的结果（其实也可以只用随机数表示）挖矿是用一个固定字符串+proof转换的字符串进行哈希的结果，如果有0就成功
		std::string previous_hash;

		bool operator == (const struct __block & value) const
		{
			return
				this->index == value.index &&
				this->timestamp == value.timestamp &&
				this->previous_hash == value.previous_hash &&
				this->lst_ts == value.lst_ts &&
				this->proof == value.proof;
		}
	} Block;


	class BlockChain
	{
	public:
		static BlockChain *Instance();//生成的区块链实例
		std::string GetJsonFromBlock(Block &block);//json文本传输数据小，便于调试扩展，区块链的数据选用它来实现
		std::string GetJsonFromTransactions(Transactions &ts);
		Block GetBlockFromJson(const std::string &json);
		Transactions GetTransactionsFromJson(const std::string &json);
		std::string GetJsonFromBlockList();//将区块链转为json
		std::string GetJsonFromTransactionsList();//将交易转为json
		// block = new Block()
		std::list<Block> GetBlockListFromJson(const std::string &json);
		void GetTransactionsListFromJson(const std::string &json);
		//从这下面开始重写:
		//公私钥对生成地址
		std::string CreateNewAddress(const KeyPair &keyPair);
		//下面的不用写
		Transactions CreateTransactions(const std::string &sender, const std::string &recipient, float amount);
		Block CreateBlock(int index, time_t timestamp, long int proof);
		int WorkloadProof(int last_proof);
		bool WorkloadVerification(int proof);
		std::string Mining(const std::string &addr);
		//
		int CheckBalances(const std::string &addr);
		void DeleteDuplicateTransactions(const Block &block);//先不写
		//
		void MergeBlockChain(const std::string &json);//没调用过，什么功能？？？

		//向区块链插入区块
		inline void InsertBlock(const Block &block)
		{
			pthread_mutex_lock(&m_mutexBlock);
			//如果区块没在链里
			if (m_lst_block.end() == std::find(m_lst_block.begin(), m_lst_block.end(), block))
			{	//上链
				m_lst_block.push_back(block);
			}
			pthread_mutex_unlock(&m_mutexBlock);
		}
		//向交易池插入交易
		inline void InsertTransactions(const Transactions &ts)//insert交易之前先锁住交易池线程
		{
			pthread_mutex_lock(&m_mutexTs);
			if (m_lst_ts.end() == std::find(m_lst_ts.begin(), m_lst_ts.end(), ts))//在交易池里查找ts失败
			{
				m_lst_ts.push_back(ts);//在交易池list尾部加入ts
			}
			pthread_mutex_unlock(&m_mutexTs);
		}

		inline Block GetLastBlock()
		{
			Block block;
			pthread_mutex_lock(&m_mutexBlock);
			block = m_lst_block.back();
			pthread_mutex_unlock(&m_mutexBlock);
			return block;
		}

	protected:
		BlockChain();
		virtual ~BlockChain();

	private:
		std::list<Transactions> m_lst_ts;//交易池
		std::list<Block> m_lst_block;///区块链list
		pthread_mutex_t m_mutexTs;//交易锁
		pthread_mutex_t m_mutexBlock;//区块链锁
	};
}

#endif	//__BLOCK_H