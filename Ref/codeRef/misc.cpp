char *strcpy(char *strDest, const char *strSrc)
{
	assert((strDest!=NULL)&&(strSrc!=NULL));	//指针入参判空断言
	char *address = strDest;					//下面要作指针移动，目的指针开始的地址保存起来返回
	while((*strDest++ = *strSrc++) != '\0');	//**注意，strDest是值传递（传进去一个指针的副本，实际上指针本身的地址是没有变）
	return address;								//返回目的指针地址，为了增加灵活性如支持链式表达
}

char *strcpy_test(char **strDest, const char *strSrc)
{
    assert((strDest!=NULL)&&(strSrc!=NULL));	
    char *address = *strDest;					
    while((*(*strDest)++ = *strSrc++) != '\0');	
    (*strDest)--;								//前面自增过的,这一步退到'\0'
    (*strDest)--;								//退到字符串最后一个字符
    return address;								
}
