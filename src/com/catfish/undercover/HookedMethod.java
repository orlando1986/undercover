package com.catfish.undercover;

import java.lang.annotation.Annotation;
import java.lang.reflect.Method;
import java.lang.reflect.Type;
import java.lang.reflect.TypeVariable;

/**
 * A method wrapper for the method you hooked, you can treat it as a normal
 * method
 * 
 * @author wzhfang
 * 
 */
public class HookedMethod {
    Method mMethod = null;
    HookedCallback mCallback = null;

    public HookedMethod(HookedCallback callback) {
        mCallback = callback;
    }

    public Object invoke(Object receiver, Object[] args) {
        return HookManager.invokeOriginalMethod(mMethod, receiver, args, mMethod.getParameterTypes(), mMethod.getReturnType());
    }

    public static interface HookedCallback {
        public Object invoke(HookedMethod method, Object receiver, Object[] args);
    }

    public TypeVariable<Method>[] getTypeParameters() {
        return mMethod.getTypeParameters();
    }

    public String toGenericString() {
        return mMethod.toGenericString();
    }

    public Type[] getGenericParameterTypes() {
        return mMethod.getGenericParameterTypes();
    }

    public Type[] getGenericExceptionTypes() {
        return mMethod.getGenericExceptionTypes();
    }

    public Type getGenericReturnType() {
        return mMethod.getGenericReturnType();
    }

    public Annotation[] getDeclaredAnnotations() {
        return mMethod.getDeclaredAnnotations();
    }

    public <A extends Annotation> A getAnnotation(Class<A> annotationType) {
        return mMethod.getAnnotation(annotationType);
    }

    public boolean isAnnotationPresent(Class<? extends Annotation> annotationType) {
        return mMethod.isAnnotationPresent(annotationType);
    }

    public Annotation[][] getParameterAnnotations() {
        return mMethod.getParameterAnnotations();
    }

    public boolean isVarArgs() {
        return mMethod.isVarArgs();
    }

    public boolean isBridge() {
        return mMethod.isBridge();
    }

    public boolean isSynthetic() {
        return mMethod.isSynthetic();
    }

    public Object getDefaultValue() {
        return mMethod.getDefaultValue();
    }

    public boolean equals(Object object) {
        return mMethod.equals(object);
    }

    public Class<?> getDeclaringClass() {
        return mMethod.getDeclaringClass();
    }

    public Class<?>[] getExceptionTypes() {
        return mMethod.getExceptionTypes();
    }

    public int getModifiers() {
        return mMethod.getModifiers();
    }

    public String getName() {
        return mMethod.getName();
    }

    public Class<?>[] getParameterTypes() {
        return mMethod.getParameterTypes();
    }

    public Class<?> getReturnType() {
        return mMethod.getReturnType();
    }

    public int hashCode() {
        return mMethod.hashCode();
    }

    public String toString() {
        return mMethod.toString();
    }
}
