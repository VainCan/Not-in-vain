static bool SendData(QString strTempPath,string sHttpDetials,int nHttpCode,struct mg_connection *nc)
{
    if(strTempPath.isEmpty())
    {
        qDebug()<<"Path is Null";
    }
    FILE *fpJpg =  fopen(strTempPath.toLocal8Bit().data(),"rb");
    qDebug()<<"path:"<<strTempPath.toLocal8Bit().data();

    if(fpJpg)
    {
        char *jpgBuffer = (char*) malloc(BUFFER_SIZE);
        fseek(fpJpg, 0L,SEEK_END);
        size_t length = ftell(fpJpg);
        rewind(fpJpg);

        if(length > BUFFER_SIZE)
        {
            mg_printf(nc, "HTTP/1.1 %d %s\r\n"
                          "Content-Type: application/x-www-form-urlencoded\r\n"
                          "Content-Length: %d\r\n"
                          "\r\n",
                      nHttpCode,sHttpDetials.c_str(), length);
            size_t intNum;
            while((intNum=fread(jpgBuffer,sizeof(char),BUFFER_SIZE,fpJpg)>0))
            {
                if(intNum!=1)
                {
                    QString strLog = "Read file is error.";
                    CSSTPHttpServerLog4CppUtility::OutputLocalLogError(strLog);
                }
                mg_send(nc, jpgBuffer, BUFFER_SIZE);
                QThread::usleep(2);
                memset(jpgBuffer,0x00,BUFFER_SIZE);
            }
        }
        else
        {
            mg_printf(nc, "HTTP/1.1 %d %s\r\n"
                          "Content-Type: application/x-www-form-urlencoded\r\n"
                          "Content-Length: %d\r\n"
                          "\r\n",
                      nHttpCode,sHttpDetials.c_str(), length);
            if(fread(jpgBuffer,sizeof(char),BUFFER_SIZE,fpJpg)!=1)
            {
                QString strLog = "Read file is error.";
                CSSTPHttpServerLog4CppUtility::OutputLocalLogError(strLog);
            }
            mg_send(nc, jpgBuffer, BUFFER_SIZE);
            memset(jpgBuffer,0x00,BUFFER_SIZE);
        }
        free(jpgBuffer);
        jpgBuffer =NULL;
        fclose(fpJpg);
        qDebug()<<"Finish Size:"<<length;
    }
    else
    {
        mg_printf(nc, "HTTP/1.1 %d %s\r\n"
                      "Content-Type: application/x-www-form-urlencoded\r\n"
                      "Content-Length: %d\r\n"
                      "\r\n",
                  404,"Not Found", 0);
        mg_send(nc, 0, BUFFER_SIZE);

        qDebug()<<"Open file error!";
        QString strLog = "Open file is error.";
        CSSTPHttpServerLog4CppUtility::OutputLocalLogError(strLog);
    }
    return true;
}