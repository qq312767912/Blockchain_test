#ifndef CRYPTOGRAPHY_H
#define CRYPTOGRAPHY_H

#include <vector>
#include <string>


namespace ShaCoin
{
	typedef struct __keydata
	{
		size_t len;
		unsigned char key[256];
	} KeyData;

	typedef struct __KeyPair
	{
		KeyData pubKey;
		KeyData priKey;
	} KeyPair;

	class Cryptography
	{
	public:
		// 基于boost的哈希值计算
		static std::string GetHash(void const* buffer, std::size_t len);

		// openssl的编码与解码
		/*Base64就是选出64个字符-小写字母a-z、大写字母A-Z、数字0-9、符号"+"、"/"等作为一个基本字符集。
		然后，其他所有符号都转换成这个字符集中的字符，便于网络传输。*/
		static std::string Base64Encode(const void*buff, int len);
		static void Base64Decode(const std::string &str64, void *outbuff, size_t outsize, size_t *outlen);

		// openssl的数字签名、验证
		static void Createkey(KeyPair &keyPair);//产生公私钥
		static bool Signature(const KeyData &priKey, const void *data, int datalen, unsigned char *sign, size_t signszie, unsigned int *signlen);
		static int Verify(const KeyData &pubkey, const char *data, int datalen, const unsigned char *sign, size_t signszie, unsigned int signlen);


		static std::string StringToLower(const std::string &str);
		static bool CompareNoCase(const std::string &strA, const std::string &strB);


		static std::vector<std::string> StringSplit(const std::string &str, const char sep);

	
		//protected:
		Cryptography();
		virtual ~Cryptography();

	private:

	};
}


#endif // CRYPTOGRAPHY_H
#pragma once
