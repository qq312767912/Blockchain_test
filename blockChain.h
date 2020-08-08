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
#define HAVE_STRUCT_TIMESPEC////�˴��Ķ�
#include <list>
#include <string>
#include <ctime>
#include <algorithm>
#include<pthread.h>////////////////�иĶ�
#include "cryptography.h"
//clear
namespace ShaCoin
{
	typedef struct __transactions//������
	{
		std::string sender;
		std::string recipient;
		float amount;

		bool operator == (const struct __transactions & value) const//����==�����������ڱȽ����������Ƿ���ͬ
		{
			return
				this->sender == value.sender &&
				this->recipient == value.recipient &&
				this->amount == value.amount;
		}
	}Transactions;

	typedef struct __block//������
	{
		int index;
		/*time_t�����������������1970��1��1��0ʱ0��0�뵽����ʱ�̵���������time()���������ȡ��*/
		time_t timestamp;
		std::list<Transactions> lst_ts;
		long int proof;//��һ�������proof+һ��������Ľ������ʵҲ����ֻ���������ʾ���ڿ�����һ���̶��ַ���+proofת�����ַ������й�ϣ�Ľ���������0�ͳɹ�
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
		static BlockChain *Instance();//���ɵ�������ʵ��
		std::string GetJsonFromBlock(Block &block);//json�ı���������С�����ڵ�����չ��������������ѡ������ʵ��
		std::string GetJsonFromTransactions(Transactions &ts);
		Block GetBlockFromJson(const std::string &json);
		Transactions GetTransactionsFromJson(const std::string &json);
		std::string GetJsonFromBlockList();//��������תΪjson
		std::string GetJsonFromTransactionsList();//������תΪjson
		// block = new Block()
		std::list<Block> GetBlockListFromJson(const std::string &json);
		void GetTransactionsListFromJson(const std::string &json);
		//�������濪ʼ��д:
		//��˽Կ�����ɵ�ַ
		std::string CreateNewAddress(const KeyPair &keyPair);
		//����Ĳ���д
		Transactions CreateTransactions(const std::string &sender, const std::string &recipient, float amount);
		Block CreateBlock(int index, time_t timestamp, long int proof);
		int WorkloadProof(int last_proof);
		bool WorkloadVerification(int proof);
		std::string Mining(const std::string &addr);
		//
		int CheckBalances(const std::string &addr);
		void DeleteDuplicateTransactions(const Block &block);//�Ȳ�д
		//
		void MergeBlockChain(const std::string &json);//û���ù���ʲô���ܣ�����

		//����������������
		inline void InsertBlock(const Block &block)
		{
			pthread_mutex_lock(&m_mutexBlock);
			//�������û������
			if (m_lst_block.end() == std::find(m_lst_block.begin(), m_lst_block.end(), block))
			{	//����
				m_lst_block.push_back(block);
			}
			pthread_mutex_unlock(&m_mutexBlock);
		}
		//���׳ز��뽻��
		inline void InsertTransactions(const Transactions &ts)//insert����֮ǰ����ס���׳��߳�
		{
			pthread_mutex_lock(&m_mutexTs);
			if (m_lst_ts.end() == std::find(m_lst_ts.begin(), m_lst_ts.end(), ts))//�ڽ��׳������tsʧ��
			{
				m_lst_ts.push_back(ts);//�ڽ��׳�listβ������ts
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
		std::list<Transactions> m_lst_ts;//���׳�
		std::list<Block> m_lst_block;///������list
		pthread_mutex_t m_mutexTs;//������
		pthread_mutex_t m_mutexBlock;//��������
	};
}

#endif	//__BLOCK_H