/* DO NOT EDIT THIS FILE - it is machine generated */
#include <jni.h>
/* Header for class com_crypt_Tea */

#ifndef _Included_com_crypt_Tea
#define _Included_com_crypt_Tea
#ifdef __cplusplus
extern "C" {
#endif
/*
 * Class:     com_crypt_Tea
 * Method:    setPassword
 * Signature: (Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_com_crypt_Tea_setPassword
  (JNIEnv *, jobject, jstring);

/*
 * Class:     com_crypt_Tea
 * Method:    encodeTextWithBase64
 * Signature: (Ljava/lang/String;)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_crypt_Tea_encodeTextWithBase64
  (JNIEnv *, jobject, jstring);

/*
 * Class:     com_crypt_Tea
 * Method:    decodeTextWithBase64
 * Signature: (Ljava/lang/String;)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_crypt_Tea_decodeTextWithBase64
  (JNIEnv *, jobject, jstring);

/*
 * Class:     com_crypt_Tea
 * Method:    encode
 * Signature: ([B)[B
 */
JNIEXPORT jbyteArray JNICALL Java_com_crypt_Tea_encode
  (JNIEnv *, jobject, jbyteArray);

/*
 * Class:     com_crypt_Tea
 * Method:    decode
 * Signature: ([B)[B
 */
JNIEXPORT jbyteArray JNICALL Java_com_crypt_Tea_decode
  (JNIEnv *, jobject, jbyteArray);

#ifdef __cplusplus
}
#endif
#endif