#ifndef _DXRENDER_CONST_DEFINE_H_
#define _DXRENDER_CONST_DEFINE_H_
#pragma once

// 以下为DxRender中方法调用返回值所代表的意义
#define DXRENDER_RESULT_OK						0		//OK，成功
#define DXRENDER_RESULT_FAILED					-20001	// 操作失败
#define DXRENDER_RESULT_NOT_INIT_YET			-20002	//未初始化
#define DXRENDER_RESULT_OPT_NOT_SUPPORT			-20003	//操作不支持
#define DXRENDER_RESULT_FUNC_REENTRY_INVALID    -20004	//调用的方法不支持重入，比如不能连续调用两次Lock，而必须是Lock、Unlock
#define DXRENDER_RESULT_PARAM_INVALID			-20005  //参数不合法
#define DXRENDER_RESULT_INTERNAL_ERR			-20006  //内部错误
#define DXRENDER_RESULT_SUBCOMPONENT_INIT_FAILED -20007 //子模块初始化失败
#define DXRENDER_RESULT_ERROR_IN_MODEL			-20008	//ElemDsplModel未设置或者状态不正确
#define DXRENDER_RESULT_EFFECT_INVALID			-20009	//Effect不可用
#define DXRENDER_RESULT_METADATA_INVALID		-200010 //DisplayElement关联的MetaData不可用
#define DXRENDER_RESULT_CREATE_BUF_FAILED		-200011 //创建Buffer资源失败
#endif //_DXRENDER_CONST_DEFINE_H_
