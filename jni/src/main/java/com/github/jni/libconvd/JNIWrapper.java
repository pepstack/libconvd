/**
 * JNIWrapper.java
 *
 *   $ cd $projectdir/jni/src/main/java
 *   $ javah -classpath . -jni com.github.jni.convd.JNIWrapper
 */
package com.github.jni.libconvd;

import com.github.jni.JNIUtils;


public class JNIWrapper {

    static {
        final String jniLibsPrefix = JNIUtils.getJniLibsPrefix(true);

        System.load(JNIUtils.concatPaths(jniLibsPrefix, "libconvd_dll.dll"));
        System.load(JNIUtils.concatPaths(jniLibsPrefix, "convd_jniwrapper.dll"));
    }


    public native void JNI_convd_lib_version();


    public static void main(String[] args) {
        JNIWrapper jniCall = new JNIWrapper();

        jniCall.JNI_convd_lib_version();
    }
}