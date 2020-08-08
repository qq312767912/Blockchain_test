#include <sstream>
#include <cstring>
#include<iostream>

#include <openssl/rsa.h>
#include <openssl/x509.h>
#include <openssl/bn.h>
#include <openssl/evp.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>
#include<algorithm>

#include "cryptography.h"


#include <boost/uuid/sha1.hpp>
#include <boost/archive/iterators/base64_from_binary.hpp>
#include <boost/archive/iterators/binary_from_base64.hpp>
#include <boost/archive/iterators/transform_width.hpp>


using namespace std;

namespace ShaCoin
{
	Cryptography::Cryptography()
	{

	}
	Cryptography::~Cryptography()
	{

	}
	//��ϣ����
	std::string Cryptography::GetHash(void const* buffer, std::size_t len)
	{
		std::stringstream ss;
		boost::uuids::detail::sha1 sha;
		sha.process_bytes(buffer, len);
		unsigned int digest[5];      //ժҪ�ķ���ֵ
		sha.get_digest(digest);
		for (int i = 0; i < 5; ++i)
			ss << std::hex << digest[i];

		return ss.str();
	}

	//�������ƵĹ�ϣֵת����base64�ַ������ַ���
	std::string Cryptography::Base64Encode(const void*buff, int len)
	{
		int i;
		std::string str;
		int outl = -1;
		char out[(1024 * 5) / 3];
		EVP_ENCODE_CTX *ctx = EVP_ENCODE_CTX_new();

		if (!ctx)
			return str;

		EVP_EncodeInit(ctx);

		for (i = 0; i < len / 1024; ++i)
		{
			memset(out, 0, sizeof(out));
			EVP_EncodeUpdate(ctx, (unsigned char *)out, &outl, (unsigned char *)buff + i * 1024, 1024);
			str += std::string(out, outl);
		}

		memset(out, 0, sizeof(out));
		EVP_EncodeUpdate(ctx, (unsigned char *)out, &outl, (unsigned char *)buff + i * 1024, len % 1024);
		str += std::string(out, outl);

		memset(out, 0, sizeof(out));
		EVP_EncodeFinal(ctx, (unsigned char *)out, &outl);
		str += std::string(out, outl);

		EVP_ENCODE_CTX_free(ctx);

		str.erase(std::remove(str.begin(), str.end(), '\n'), str.end());

		return str;
	}

	// openssl�Ľ���
	void Cryptography::Base64Decode(const std::string &str64, void *outbuff, size_t outsize, size_t *outlen)
	{
		unsigned int i;
		unsigned int inlen = str64.length();
		if (outsize * 5 / 3 < inlen)
		{
			*outlen = -1;
			return;
		}

		int outl = -1;
		char out[(1024 * 5) / 3];
		char *p = (char *)outbuff;

		EVP_ENCODE_CTX *ctx = EVP_ENCODE_CTX_new();

		if (!ctx)
		{
			*outlen = -1;
			return;
		}

		EVP_DecodeInit(ctx);

		for (i = 0; i < str64.length() / 1024; ++i)
		{
			memset(out, 0, sizeof(out));
			EVP_DecodeUpdate(ctx, (unsigned char *)out, &outl, (unsigned char *)str64.c_str() + i * 1024, 1024);
			memcpy(p, out, outl);
			p += outl;
			*outlen += outl;
		}

		memset(out, 0, sizeof(out));
		EVP_DecodeUpdate(ctx, (unsigned char *)out, &outl, (unsigned char *)str64.c_str() + i * 1024, str64.length() % 1024);
		memcpy(p, out, outl);
		p += outl;
		*outlen += outl;

		memset(out, 0, sizeof(out));
		EVP_DecodeFinal(ctx, (unsigned char *)out, &outl);
		memcpy(p, out, outl);
		p += outl;
		*outlen += outl;
	}

	// ���������˽Կ��
	void Cryptography::Createkey(KeyPair &keyPair)
	{
		unsigned char *p = NULL;
		keyPair.priKey.len = -1;
		keyPair.pubKey.len = -1;

		EC_GROUP *group = EC_GROUP_new_by_curve_name(NID_secp256k1);//ѡ���������
		if (!group)
			return;
		//1������EC_KEY *key�ṹ����Բ���ߵĲ�������˽Կ�������ڴ˽ṹ��
		//2��EC_GROUP�ṹ������Բ���ߵĲ���
		EC_KEY *key = EC_KEY_new();//����һ��EC_KEY�ṹ
		if (!key)
		{
			EC_GROUP_free(group);
			return;
		}

		if (!EC_KEY_set_group(key, group))//��EC_GROUP group�е�������䵽EC_KEY key�У����û��д��ȥ�����ͷ�
		{
			EC_GROUP_free(group);
			EC_KEY_free(key);
			return;
		}

		if (!EC_KEY_generate_key(key))//���ɹ�˽Կ������key��
		{
			EC_GROUP_free(group);
			EC_KEY_free(key);
			return;
		}

		if (!EC_KEY_check_key(key))
		{
			EC_GROUP_free(group);
			EC_KEY_free(key);
			return;
		}

		keyPair.priKey.len = i2d_ECPrivateKey(key, NULL);
		if (keyPair.priKey.len > (int)sizeof(keyPair.priKey.key))
		{
			keyPair.priKey.len = -1;
			EC_GROUP_free(group);
			EC_KEY_free(key);
			return;
		}
		p = keyPair.priKey.key;
		keyPair.priKey.len = i2d_ECPrivateKey(key, &p);//��˽Կ���ݵ���˽Կ�ṹ��


		keyPair.pubKey.len = i2o_ECPublicKey(key, NULL);
		if (keyPair.pubKey.len > (int)sizeof(keyPair.pubKey.key))
		{
			keyPair.pubKey.len = -1;
			EC_GROUP_free(group);
			EC_KEY_free(key);
			return;
		}
		p = keyPair.pubKey.key;
		keyPair.pubKey.len = i2o_ECPublicKey(key, &p);

		EC_GROUP_free(group);
		EC_KEY_free(key);
	}

	//��dataǩ������sign��
	bool Cryptography::Signature(const KeyData &priKey, const void *data, int datalen, unsigned char *sign, size_t signszie, unsigned int *signlen)
	{
		EC_KEY *ec_key = NULL;
		const unsigned char *pp = (const unsigned char *)priKey.key;
		ec_key = d2i_ECPrivateKey(&ec_key, &pp, priKey.len);
		if (!ec_key)
			return false;

		if (ECDSA_size(ec_key) > (int)signszie)
		{
			EC_KEY_free(ec_key);
			return false;
		}

		if (!ECDSA_sign(0, (unsigned char *)data, datalen, sign, signlen, ec_key))
		{
			EC_KEY_free(ec_key);
			return false;
		}

		EC_KEY_free(ec_key);
		return true;
	}

	int Cryptography::Verify(const KeyData &pubkey, const char *data, int datalen, const unsigned char *sign, size_t signszie, unsigned int signlen)
	{
		int ret = -1;
		EC_KEY *ec_key = NULL;
		EC_GROUP *ec_group = NULL;
		const unsigned char *pp = (const unsigned char *)pubkey.key;

		ec_key = EC_KEY_new();
		if (!ec_key)
			return ret;

		if (ECDSA_size(ec_key) > (int)signszie)
		{
			EC_KEY_free(ec_key);
			return ret;
		}

		ec_group = EC_GROUP_new_by_curve_name(NID_secp256k1);
		if (!ec_group)
		{
			EC_KEY_free(ec_key);
			return ret;
		}

		if (!EC_KEY_set_group(ec_key, ec_group))
		{
			EC_GROUP_free(ec_group);
			EC_KEY_free(ec_key);
			return ret;
		}

		ec_key = o2i_ECPublicKey(&ec_key, &pp, pubkey.len);
		if (!ec_key)
		{
			EC_GROUP_free(ec_group);
			EC_KEY_free(ec_key);
			return ret;
		}
		//data��sign��֤
		ret = ECDSA_verify(0, (const unsigned char*)data, datalen, sign,
			signlen, ec_key);

		EC_GROUP_free(ec_group);
		EC_KEY_free(ec_key);
		return ret;
	}

	std::string Cryptography::StringToLower(const std::string &str)
	{
		std::string strTmp = str;
		std::transform(strTmp.begin(), strTmp.end(), strTmp.begin(), ::tolower);
		return strTmp;
	}

	//�Ƚ������ַ����Ƿ���ͬ
	bool Cryptography::CompareNoCase(const std::string &strA, const std::string &strB)
	{
		std::string str1 = StringToLower(strA);
		std::string str2 = StringToLower(strB);
		return (str1 == str2);
	}

	//��str�����sep���õ�ʱ���ǿո񣩷ָ��Ӵ���������
	std::vector<std::string> Cryptography::StringSplit(const std::string &str, const char sep)
	{
		std::vector<std::string> strvec;
		//���ڱ���string���͵ĳ���
		std::string::size_type pos1, pos2;
		//pos2ָ���ҵ��ո��λ��
		pos2 = str.find(sep);
		//pos1ָ���ʼλ��
		pos1 = 0;
		//����ֵΪĿ���ַ���λ�ã���û���ҵ�Ŀ���ַ�ʱ����npos
		while (std::string::npos != pos2)
		{
			strvec.push_back(str.substr(pos1, pos2 - pos1));

			pos1 = pos2 + 1;
			pos2 = str.find(sep, pos1);
		}
		strvec.push_back(str.substr(pos1));
		return strvec;

	}

}
