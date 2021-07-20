/**
 * convd_jniwrapper.c
 */
#include <com_github_jni_libconvd_JNIWrapper.h>

#include <convd/convd_api.h>


void JNICALL Java_com_github_jni_libconvd_JNIWrapper_JNI_1convd_1lib_1version(JNIEnv *env, jobject obj)
{
    const char *libname;
    const char * libversion = convd_lib_version(&libname);

    printf("[%s:%d %s] %s-%s\n", __FILE__, __LINE__, __FUNCTION__, libname, libversion);
}