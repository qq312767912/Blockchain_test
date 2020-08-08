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
		// ����boost�Ĺ�ϣֵ����
		static std::string GetHash(void const* buffer, std::size_t len);

		// openssl�ı��������
		/*Base64����ѡ��64���ַ�-Сд��ĸa-z����д��ĸA-Z������0-9������"+"��"/"����Ϊһ�������ַ�����
		Ȼ���������з��Ŷ�ת��������ַ����е��ַ����������紫�䡣*/
		static std::string Base64Encode(const void*buff, int len);
		static void Base64Decode(const std::string &str64, void *outbuff, size_t outsize, size_t *outlen);

		// openssl������ǩ������֤
		static void Createkey(KeyPair &keyPair);//������˽Կ
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
