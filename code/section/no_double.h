#ifndef NO_DOUBLE_H
#define NO_DOUBLE_H
// 方法1：使用预处理器重定义（编译时错误）
#ifdef __CC_ARM               // ARMCC编译器
    #define double ERROR_DOUBLE_TYPE_NOT_ALLOWED_USE_FLOAT_INSTEAD
    #define DOUBLE ERROR_DOUBLE_TYPE_NOT_ALLOWED_USE_FLOAT_INSTEAD
#elif defined(__ARMCC_VERSION) // ARMClang编译器
    #define double ERROR_DOUBLE_TYPE_NOT_ALLOWED_USE_FLOAT_INSTEAD
    #define DOUBLE ERROR_DOUBLE_TYPE_NOT_ALLOWED_USE_FLOAT_INSTEAD
#endif

//// 方法2：静态断言（C11）
//#if __STDC_VERSION__ >= 201112L
//    // 如果检测到double类型，静态断言失败
//    // 注意：这需要实际使用double才能触发
//#endif

//// 定义单精度替代
//typedef float float32_t;
//#define PI          3.14159265358979323846f
//#define DOUBLE(x)   ((float)(x))  // 强制转换宏

#endif // NO_DOUBLE_H

