/*
 * File:        CryptoUtils.hpp
 * Author:      summer@SummerLab
 * CreateDate:  2024-03-27
 * LastEdit:    2024-03-27
 * Description: Crypto Utils for Linux & StockTrader
 */

#pragma once

#include <string>
#include <string.h>

#include <openssl/sha.h>
#include <openssl/aes.h>
#include <openssl/hmac.h>


////////////////////////////////////////////////////////
///                                                  ///
///                   Crypto工具集合                  ///
///                                                  ///
////////////////////////////////////////////////////////
namespace CryptoUtils
{
    /************************************************************************/
    // 函数名:   GetHMAC_SHA256
    // 函数功能: 生成基于秘钥的SHA256摘要哈希 (哈希指纹)
    // 函数说明: 
    /************************************************************************/ 
    static inline std::string GetHMAC_SHA256(const std::string &secretKey, const std::string &data)
    {
        unsigned char* digest;
        digest = HMAC(EVP_sha256(), secretKey.c_str(), secretKey.length(), \
                      reinterpret_cast<const unsigned char*>(data.c_str()), data.length(), NULL, NULL);
        char mdString[65];
        for(int i = 0; i < 32; i++)
            sprintf(&mdString[i*2], "%02x", (unsigned int)digest[i]);
        std::string strHMACSHA256 = std::string(mdString);
        return strHMACSHA256;
    }

    /************************************************************************/
    // 函数名:   GetSHA256
    // 函数功能: 生成SHA256摘要哈希 (哈希指纹)
    // 函数说明: 
    /************************************************************************/ 
    static inline std::string GetSHA256(const std::string &strData)
    {
        // 声明返回哈希值 (256/sizeof(byte))+1
        unsigned char chHV[33] = {0};
        SHA256((const unsigned char *)strData.c_str(), strData.length(), chHV);
        std::string strSHA256 = std::string((const char *)chHV);
        return strSHA256;
    }

#if 0
    /************************************************************************/
    // 函数名:     GetSHA256Hex
    // 函数功能:   生成十六进制SHA256摘要哈希 (哈希指纹)
    // 函数参数:   待处理字符串
    // 函数返回值: 十六进制256位摘要哈希
    /************************************************************************/ 
    static inline std::string GetSHA256Hex(const std::string &strData)
    {
        // 声明返回哈希值
        unsigned char chHV[33] = {0};
        // 调用SHA256接口
        SHA256((const unsigned char *)strData.c_str(), strData.length(), chHV);

        // 转换十六进制
        char buf[65] = {0};
        char tmp[3] = {0};
        for( int i=0; i<32; i++ )
        {
            sprintf(tmp, "%02x", chHV[i]);
            strcat(buf, tmp);
        }
        // 从32字节截断
        buf[32] = '\0';

        // 返回十六进制256位摘要哈希
        std::string strSHA256Hex = std::string(buf);
        return strSHA256Hex;
    }

    /************************************************************************/
    // 函数名:     DecodeAES256CBC
    // 函数功能:   AES解密 (AES256标准，CBC模式)
    // 函数参数:   密文+秘钥
    // 函数返回值: 原文
    /************************************************************************/
    static inline std::string DecodeAES256CBC(const std::string& strData, const std::string& strKey)
    {
        // tips summer@20230112 - 考虑是否将IV传参
        // 默认将iv全置为'0'
        unsigned char iv[AES_BLOCK_SIZE] = {'0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0'};

        // 设置AES解密秘钥
        AES_KEY aes_key;
        if (AES_set_decrypt_key((const unsigned char*)strKey.c_str(), strKey.length()*8, &aes_key) < 0)
        {
            //assert(false);
            return "";
        }

        // AES解密 (CBC模式)
        std::string strRet;
        for( unsigned int i = 0; i < strData.length()/AES_BLOCK_SIZE; i++ )
        {
            std::string str16 = strData.substr(i*AES_BLOCK_SIZE, AES_BLOCK_SIZE);
            unsigned char out[AES_BLOCK_SIZE];
            ::memset(out, 0, AES_BLOCK_SIZE);
            AES_cbc_encrypt((const unsigned char*)str16.c_str(), out, AES_BLOCK_SIZE, &aes_key, iv, AES_DECRYPT);
            strRet += std::string((const char*)out, AES_BLOCK_SIZE);
        }
        return strRet;
    }

    /************************************************************************/
    //   Base64字符集
    /************************************************************************/
    static const std::string Base64Chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    /************************************************************************/
    // 函数名:     IsBase64
    // 函数功能:   判断字节是否包含于Base64字符集
    // 函数参数:   
    // 函数返回值: 
    /************************************************************************/
    static inline bool IsBase64(unsigned char c)
    {
        return ( isalnum(c) || (c == '+') || (c == '/') );
    }

    /************************************************************************/
    // 函数名:     DecodeBase64
    // 函数功能:   Base64解码
    // 函数参数:   Base64字符串
    // 函数返回值: 原始字符串
    /************************************************************************/
    static inline std::string DecodeBase64(const std::string& strData)
    {
        int in_len = strData.size();
        int i = 0;
        int j = 0;
        int in_ = 0;
        unsigned char char_array_4[4], char_array_3[3];
        std::string strRet;

        while( in_len-- && (strData[in_] != '=') && IsBase64(strData[in_]) )
        {
            char_array_4[i++] = strData[in_];
            in_++;
            if( i == 4 )
            {
                for (i = 0; i < 4; i++)
                    char_array_4[i] = Base64Chars.find(char_array_4[i]);

                char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
                char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
                char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

                for (i = 0; (i < 3); i++)
                    strRet += char_array_3[i];
                
                i = 0;
            }
        }

        if( i )
        {
            for (j = i; j < 4; j++)
                char_array_4[j] = 0;
            for (j = 0; j < 4; j++)
                char_array_4[j] = Base64Chars.find(char_array_4[j]);

            char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
            char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
            char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

            for (j = 0; (j < i - 1); j++)
                strRet += char_array_3[j];
        }

        return strRet;
    }

    /************************************************************************/
    // 函数名:     Decrypt
    // 函数功能:   解密(Python)Encrypt加密后的密文
    // 函数参数:   
    // 函数返回值: 
    /************************************************************************/
    static inline std::string Decrypt(const std::string& strData, const std::string& strKey)
    {
        std::string strBase = DecodeBase64(strData); 
        std::string strSHAKey = GetSHA256(strKey);
        std::string strRet = DecodeAES256CBC(strBase, strSHAKey);
        int retlen = strRet.length() - AES_BLOCK_SIZE - (int)strRet[strRet.length()-1];
        return strRet.substr(AES_BLOCK_SIZE, retlen);
    }
#endif
}
