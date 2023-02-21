#include "classloader.h"
#include "../JNIHelper/JNIHelper.hpp"
#include "art.h"
#include "plt.h"
#include "xposed.h"
#include "hash.h"
#include "Utils.h"
#include <string.h>


jobject newLocalRef(JNIEnv *env, void *object) {
    static jobject (*NewLocalRef)(JNIEnv *, void *) = nullptr;
    if (object == nullptr) {
        return nullptr;
    }
    if (NewLocalRef == nullptr) {
        NewLocalRef = (jobject (*)(JNIEnv *, void *)) plt_dlsym(
                "_ZN3art9JNIEnvExt11NewLocalRefEPNS_6mirror6ObjectE", nullptr);
        //LOGD("NewLocalRef: %p", NewLocalRef);
    }
    if (NewLocalRef != nullptr) {
        return NewLocalRef(env, object);
    } else {
        return nullptr;
    }
}

void deleteLocalRef(JNIEnv *env, jobject object) {
    static void (*DeleteLocalRef)(JNIEnv *, jobject) = nullptr;
    if (DeleteLocalRef == nullptr) {
        DeleteLocalRef = (void (*)(JNIEnv *, jobject)) plt_dlsym(
                "_ZN3art9JNIEnvExt14DeleteLocalRefEP8_jobject", nullptr);
        //LOGD("DeleteLocalRef: %p", DeleteLocalRef);
    }
    if (DeleteLocalRef != nullptr) {
        DeleteLocalRef(env, object);
    }
}


//获取object class的描述
char* getObjectdescription(JNIEnv* env, jobject obj){
    char* desc = nullptr;

    jclass objClazz = env->GetObjectClass(obj);
    jclass clazz = env->FindClass("java/lang/Class");
    jmethodID getClassMethod = env->GetMethodID(objClazz, "getClass", "()Ljava/lang/Class;");
    jclass finalClazz = (jclass)env->CallObjectMethod(obj, getClassMethod);
    jmethodID getNameMethod = env->GetMethodID(clazz, "getName", "()Ljava/lang/String;");
    jstring jname = (jstring)env->CallObjectMethod(finalClazz, getNameMethod);

    const char* name = env->GetStringUTFChars(jname, NULL);
    if(name != NULL){
        desc = strdup(name);
    }
    env->ReleaseStringUTFChars(jname, name);

    env->DeleteLocalRef(jname);
    env->DeleteLocalRef(finalClazz);
    env->DeleteLocalRef(clazz);
    env->DeleteLocalRef(objClazz);

    return desc;
}


class ClassLoaderVisitor : public art::SingleRootVisitor {
public:
    ClassLoaderVisitor(C_JNIEnv *env, jclass classLoader) : env_(env), classLoader_(classLoader) {
        mClassCnt = 0;
    }

    void VisitRoot(art::mirror::Object *root, const art::RootInfo &info ATTRIBUTE_UNUSED) final {
        LOGE("in VisitRoot");
        jobject object = newLocalRef((JNIEnv *) env_, (jobject) root);
        if (object != nullptr) {
            const char* desc = getObjectdescription((JNIEnv *) env_, object);
            LOGE("in VisitRoot object != nullptr,%s",desc);
            mClassCnt++;
            if ((*env_)->IsInstanceOf((JNIEnv *) env_, object, classLoader_)) {
                xposed::doAntiXposed(env_, object, (intptr_t) root);
            }
            deleteLocalRef((JNIEnv *) env_, object);
        }
    }

    int getClassNumber(){
        return mClassCnt;
    }

private:
    C_JNIEnv *env_;
    jclass classLoader_;
    int mClassCnt;
};

void classloader::checkGlobalRef(C_JNIEnv *env, jclass clazz) {
    auto VisitRoots = (void (*)(void *, void *)) plt_dlsym(
            "_ZN3art9JavaVMExt10VisitRootsEPNS_11RootVisitorE", nullptr);

    if (VisitRoots == nullptr) {
        return;
    }
    JavaVM *jvm;
    (*env)->GetJavaVM((JNIEnv *) env, &jvm);
    ClassLoaderVisitor visitor(env, clazz);
    VisitRoots(jvm, &visitor);
    LOGE("checkGlobalRef, class number:%d",visitor.getClassNumber());
}

class WeakClassLoaderVisitor : public art::IsMarkedVisitor {
public:
    WeakClassLoaderVisitor(C_JNIEnv *env, jclass classLoader) : env_(env),
                                                                classLoader_(classLoader) {
    }

    art::mirror::Object *IsMarked(art::mirror::Object *obj) override {
        LOGE("in IsMarked");
        jobject object = newLocalRef((JNIEnv *) env_, (jobject) obj);
        if (object != nullptr) {
            LOGE("in IsMarked object != nullptr");
            if ((*env_)->IsInstanceOf((JNIEnv *) env_, object, classLoader_)) {
                xposed::doAntiXposed(env_, object, (intptr_t) obj);
            }
            deleteLocalRef((JNIEnv *) env_, object);
        }
        return obj;
    }

private:
    C_JNIEnv *env_;
    jclass classLoader_;
};

void classloader::checkWeakGlobalRef(C_JNIEnv *env, jclass clazz) {
    auto SweepJniWeakGlobals = (void (*)(void *, void *)) plt_dlsym(
            "_ZN3art9JavaVMExt19SweepJniWeakGlobalsEPNS_15IsMarkedVisitorE", nullptr);
    if (SweepJniWeakGlobals == nullptr) {
        return;
    }
    JavaVM *jvm;
    (*env)->GetJavaVM((JNIEnv *) env, &jvm);
    WeakClassLoaderVisitor visitor(env, clazz);
    SweepJniWeakGlobals(jvm, &visitor);
}

void classloader::checkClassLoader(C_JNIEnv *env, int sdk) {
    if (sdk < 21) {
        return;
    }

    jclass clazz = (*env)->FindClass((JNIEnv *) env, "dalvik/system/BaseDexClassLoader");
    if ((*env)->ExceptionCheck((JNIEnv *) env)) {
        (*env)->ExceptionClear((JNIEnv *) env);
    }

    if (clazz == nullptr) {
        return;
    }

    checkGlobalRef(env, clazz);
    checkWeakGlobalRef(env, clazz);

    clear();
    (*env)->DeleteLocalRef((JNIEnv *) env, clazz);

}