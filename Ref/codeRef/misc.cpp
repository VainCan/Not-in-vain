char *strcpy(char *strDest, const char *strSrc)
{
	assert((strDest!=NULL)&&(strSrc!=NULL));	//ָ������пն���
	char *address = strDest;					//����Ҫ��ָ���ƶ���Ŀ��ָ�뿪ʼ�ĵ�ַ������������
	while((*strDest++ = *strSrc++) != '\0');	//**ע�⣬strDest��ֵ���ݣ�����ȥһ��ָ��ĸ�����ʵ����ָ�뱾��ĵ�ַ��û�б䣩
	return address;								//����Ŀ��ָ���ַ��Ϊ�������������֧����ʽ���
}

char *strcpy_test(char **strDest, const char *strSrc)
{
    assert((strDest!=NULL)&&(strSrc!=NULL));	
    char *address = *strDest;					
    while((*(*strDest)++ = *strSrc++) != '\0');	
    (*strDest)--;								//ǰ����������,��һ���˵�'\0'
    (*strDest)--;								//�˵��ַ������һ���ַ�
    return address;								
}
