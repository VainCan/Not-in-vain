
#ifdef Q_OS_WIN
#include <windows.h>
#include <Winternl.h>
#include <tlhelp32.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <Iphlpapi.h>
#include <conio.h>
#endif
#include "Monitor.h"
#ifdef Q_OS_WIN
#define MALLOC(x) HeapAlloc(GetProcessHeap(), 0, (x))
#define FREE(x) HeapFree(GetProcessHeap(), 0, (x))
#endif
#include <cpuid.h>
#include <iostream>
#include <qcoreapplication.h>
#include <QStringList>
#include <QThread>
#include <stdio.h>
#include <QDebug>
#include <QCryptographicHash>
#include <QFile>



QString CMonitor::m_sEthName = "eth";

CMonitor::CMonitor()
{
    m_nOldCpuToalTime = 0;
    m_nOldCpuIdleTime = 0;
    m_nOldProgressCpuTime = 0;
    m_nOldNetRecv = 0;
    m_nOldNetSend = 0;

    foreach(QString sCmd,qApp->arguments())
    {
        QRegExp regex("\\[EthernetName!([^\\]]{1,})\\]");
        int nPos;
        if( (nPos = sCmd.indexOf(regex)) >= 0 && regex.captureCount() == 1)
        {
            CMonitor::m_sEthName = regex.cap(1);
            break;
        }
    }

#ifdef Q_OS_WIN
    GetMacAddress( m_sMacAddress );
#endif
    GetCpuUsed();
    GetNetUsed();
    QThread::msleep(100);
}

#ifdef Q_OS_WIN
double FileTimeToDouble(FILETIME &filetime)
{
    return (double)(filetime.dwHighDateTime * 4.294967296E9) + (double)filetime.dwLowDateTime;
}
#endif

bool CMonitor::GetMacAddress(QString &strMac)
{
#ifdef Q_OS_WIN
    PIP_ADAPTER_INFO pAdapterInfo,pAdapterNext;
    DWORD AdapterInfoSize;
    char szMac[32]   =   {0};
    DWORD Err;AdapterInfoSize = 0;
    Err = GetAdaptersInfo(NULL,   &AdapterInfoSize);
    if((Err != 0)&&(Err != ERROR_BUFFER_OVERFLOW))
    {
        return false;
    }
    pAdapterInfo = (PIP_ADAPTER_INFO) GlobalAlloc(GPTR, AdapterInfoSize);
    if(pAdapterInfo == NULL)
    {
        return false;
    }
    if(GetAdaptersInfo(pAdapterInfo,   &AdapterInfoSize)   !=   0)
    {
        GlobalFree(pAdapterInfo);
        return false;
    }

    pAdapterNext = pAdapterInfo;
    while(pAdapterNext)
    {
        strMac.clear();
        for(int i = 0 ; i < pAdapterNext->AddressLength;++i)
        {
            strMac += QString::number(pAdapterNext->Address[i],16).toUpper();
            if(i != pAdapterNext->AddressLength-1)
                strMac += "-";
        }
        if(!strMac.isEmpty())
            break;

        pAdapterNext = pAdapterNext->Next;
    }

    GlobalFree(pAdapterInfo);
#elif defined(Q_OS_LINUX)
    //修改地址
    QString cmd = QString("ifconfig |grep eth");
    // QString cmd = QString("ip adder |grep eth");
    QByteArray sShellResult;
    FILE *fp = NULL;
    fp = popen(cmd.toStdString().c_str(),"r");
    if(fp)
    {
        char buf[100] = {0};
        int read = 0;
        while((read = fread(buf, sizeof(char),100, fp) ) != 0)
        {
            sShellResult.append(buf,read);
        }
        pclose(fp);
        if(sShellResult.indexOf("eth") >= 0)
        {
            QRegExp regex("([0-9a-fA-F:]{17})");
            int nPos;
            if( (nPos = QString(sShellResult).indexOf(regex)) >= 0 && regex.captureCount() == 1)
            {
                strMac = regex.cap(1);
            }
        }
    }
#endif
    return !strMac.isEmpty();
}


double CMonitor::GetCpuUsed()
{
    double nCpuUsed = 1;
#ifdef Q_OS_LINUX
    QString cmd = QString("cat /proc/stat |grep cpu[^0-9]");
    QByteArray sShellResult;
    FILE *fp = NULL;
    fp = popen(cmd.toStdString().c_str(),"r");
    if(fp)
    {
        char buf[100] = {0};
        int read = 0;
        while((read = fread(buf, sizeof(char),100, fp) ) != 0)
        {
            sShellResult.append(buf,read);
        }
        pclose(fp);

        long long idle,TotalCpu = 0;
        if(sShellResult.startsWith("cpu  "))
        {
            sShellResult.remove(0,5);
            QStringList sList = QString(sShellResult).split(' ');
            if(sList.length() >= 7)
            {
                idle = sList[3].toLongLong();
                foreach (QString item, sList)
                    TotalCpu += item.toLongLong();

                if(m_nOldCpuToalTime != 0 && m_nOldCpuIdleTime != 0)
                {
                    nCpuUsed = (1 - ( idle - m_nOldCpuIdleTime)/(double)(TotalCpu - m_nOldCpuToalTime)) * 100;
                }
                m_nOldCpuToalTime = TotalCpu;
                m_nOldCpuIdleTime = idle;
            }
        }
    }
#elif defined(Q_OS_WIN)
    FILETIME ftIdle, ftKernel, ftUser;
    if (GetSystemTimes(&ftIdle, &ftKernel, &ftUser))
    {
        double fCPUIdleTime = FileTimeToDouble(ftIdle);
        double fCPUKernelTime = FileTimeToDouble(ftKernel);
        double fCPUUserTime = FileTimeToDouble(ftUser);

        if( m_nOldCpuToalTime != 0 && m_nOldCpuIdleTime != 0 )
        {
            nCpuUsed = (100.0 - (fCPUIdleTime - m_nOldCpuIdleTime)
                        / (fCPUKernelTime + fCPUUserTime - m_nOldCpuToalTime)
                        *100.0);
        }
        m_nOldCpuToalTime = fCPUKernelTime + fCPUUserTime;
        m_nOldCpuIdleTime = fCPUIdleTime;
    }
#endif

    return nCpuUsed;
}

double CMonitor::GetNetUsed()
{
    double nNetUsed = 0;
#ifdef Q_OS_LINUX
    QString cmd = QString("cat /proc/net/dev|grep "+ m_sEthName);
    QByteArray sShellResult;
    FILE *fp = NULL;
    fp = popen(cmd.toStdString().c_str(),"r");
    if(fp)
    {
        char buf[100] = {0};
        int read = 0;
        while((read = fread(buf, sizeof(char),100, fp) ) != 0)
        {
            sShellResult.append(buf,read);
        }
        pclose(fp);

        long long nRecv,nSend;
        if(sShellResult.indexOf(m_sEthName) >= 0)
        {
            QStringList sList = QString(sShellResult).split(' ',QString::SkipEmptyParts);
            if(sList.length() >= 10)
            {
                nRecv = sList[1].toLongLong();
                nSend = sList[9].toLongLong();

                if(m_nOldNetRecv != 0 && m_nOldNetSend != 0)
                {
                    int nTime = m_NetCheck.elapsed();
                    nNetUsed = (nRecv - m_nOldNetRecv) + (nSend - m_nOldNetSend) * 1000 / nTime;
                }
                else
                {
                    m_NetCheck.start();
                    QThread::msleep(100);
                }

                m_nOldNetRecv = nRecv;
                m_nOldNetSend = nSend;
            }
        }
    }
#elif defined(Q_OS_WIN)
    MIB_IFTABLE *pIfTable            = (MIB_IFTABLE *) MALLOC(sizeof (MIB_IFTABLE));
    DWORD dwSize            = sizeof (MIB_IFTABLE);
    DWORD dwRet = 0;
    if(GetIfTable(pIfTable, &dwSize, FALSE) == ERROR_INSUFFICIENT_BUFFER) {
        FREE(pIfTable);
        pIfTable            = (MIB_IFTABLE *) MALLOC(dwSize);
        if (pIfTable == NULL) {
            return false;
        }
    }
    dwRet = GetIfTable(pIfTable,&dwSize,FALSE);
    if ( dwRet == NO_ERROR )
    {
        __int64                 nRecv  = 0;
        __int64                 nSend  = 0;
        for (DWORD i=0; i < pIfTable->dwNumEntries; i++ )
        {
            MIB_IFROW *pIfRow = (MIB_IFROW *) & pIfTable->table[i];

            QString sMac;
            for (int j = 0; j < pIfRow->dwPhysAddrLen; j++) {
                sMac += QString::number(pIfRow->bPhysAddr[j],16).toUpper();
                if(j != pIfRow->dwPhysAddrLen-1)
                    sMac += "-";
            }
            //if(m_sMacAddress == sMac)
            {
                nSend      += pIfTable->table[i].dwOutOctets;
                nRecv      += pIfTable->table[i].dwInOctets;
            }
        }
        if(m_nOldNetRecv != 0 && m_nOldNetSend != 0)
        {
            int nTime = m_NetCheck.elapsed();m_NetCheck.start();
            nNetUsed = (nRecv - m_nOldNetRecv) + (nSend - m_nOldNetSend) * 1000 / nTime;
            if(nNetUsed < 0)
            {
                nNetUsed = -nNetUsed;
            }
        }
        else
        {
            m_NetCheck.start();
        }
        m_nOldNetSend            = nSend;
        m_nOldNetRecv            = nRecv;
    }
    FREE(pIfTable);
#endif

    return nNetUsed;
}

double CMonitor::GetMemUsed()
{
    double nMemUsed = 1;
#ifdef Q_OS_LINUX
    QString cmd = QString("cat /proc/meminfo |grep Mem");
    QByteArray sShellResult;
    FILE *fp = NULL;
    fp = popen(cmd.toStdString().c_str(),"r");
    if(fp)
    {
        char buf[100] = {0};
        int read = 0;
        while((read = fread(buf, sizeof(char),100, fp) ) != 0)
        {
            sShellResult.append(buf,read);
        }
        pclose(fp);

        long long nMemFree,nMemTotal;
        nMemFree = nMemTotal =0;
        int nPos = 0;
        if(sShellResult.startsWith("MemTotal"))
        {
            QRegExp regex("([0-9]{1,})");
            if( (nPos = QString(sShellResult).indexOf(regex)) >= 0 && regex.captureCount() == 1)
            {
                nPos += regex.cap(1).length();
                nMemTotal = regex.cap(1).toLongLong();
                if((nPos = QString(sShellResult).indexOf(regex,nPos)) >= 0 && regex.captureCount() == 1)
                {
                    nMemFree = regex.cap(1).toLongLong();
                }
                nMemUsed = (nMemTotal - nMemFree)  * 100 / nMemTotal;
            }
        }
    }
#elif defined(Q_OS_WIN)

    MEMORYSTATUSEX statex;
    statex.dwLength = sizeof (statex);
    GlobalMemoryStatusEx (&statex);
    nMemUsed = (statex.ullTotalPhys - statex.ullAvailPhys)  * 100 / statex.ullTotalPhys;
#endif

    return nMemUsed;
}


double CMonitor::GetDiskUsed()
{
    double nDiskUsed = 1;
#ifdef Q_OS_LINUX
    QString cmd = QString("iostat -d -x|grep sda");
    QByteArray sShellResult;
    FILE *fp = NULL;
    fp = popen(cmd.toStdString().c_str(),"r");
    if(fp)
    {
        char buf[100] = {0};
        int read = 0;
        while((read = fread(buf, sizeof(char),100, fp) ) != 0)
        {
            sShellResult.append(buf,read);
        }
        pclose(fp);
        int nPos = 0;
        if(sShellResult.startsWith("sda"))
        {
            QRegExp regex("([0-9.]{1,})\n$");
            if( (nPos = QString(sShellResult).indexOf(regex)) >= 0 && regex.captureCount() == 1)
                nDiskUsed = regex.cap(1).toDouble();
        }
    }
#elif defined(Q_OS_WIN)

#endif

    return nDiskUsed;
}

QString CMonitor::GetBoardID()
{
    QString strBoard = "";
#ifdef Q_OS_LINUX
    QString cmd = QString("dmidecode -s baseboard-serial-number");
    QByteArray sShellResult;
    FILE *fp = NULL;
    fp = popen(cmd.toStdString().c_str(),"r");
    if(fp)
    {
        char buf[100] = {0};
        int read = 0;
        while((read = fread(buf, sizeof(char),100, fp) ) != 0)
        {
            sShellResult.append(buf,read);
        }
        pclose(fp);
        if(sShellResult.indexOf("Permission denied") < 0 && sShellResult.length() > 6)
        {
            strBoard = sShellResult.data();
        }
    }
#elif defined(Q_OS_WIN)

#endif
    return strBoard;
}

QString CMonitor::GetCpuID()
{
    unsigned int s1,s2,s3,s4;
    s1 = s2 = s3 = s4 =0;
    //#ifdef Q_OS_LINUX
    __get_cpuid(0,&s1,&s2,&s3,&s4);
    //#elif defined(Q_OS_WIN)
    //    asm volatile
    //    (
    //    "movl $0x01 , %%eax ;\n\t"
    //    "xorl %%edx , %%edx ;\n\t"
    //    "cpuid ;\n\t"
    //    "movl %%edx ,%0 ;\n\t"
    //    "movl %%eax ,%1 ;\n\t"
    //    :"=m"(s1),"=m"(s2)
    //    );
    //    asm volatile
    //    (
    //    "movl $0x03 , %%eax ;\n\t"
    //    "xorl %%ecx , %%ecx ;\n\t"
    //    "xorl %%edx , %%edx ;\n\t"
    //    "cpuid ;\n\t"
    //    "movl %%edx ,%0 ;\n\t"
    //    "movl %%ecx ,%1 ;\n\t"
    //    :"=m"(s3),"=m"(s4)
    //    );
    //#endif
    QString cpu;
    cpu = "%1%2%3%4";
    cpu = cpu.arg(s1,8,16,QChar('0')).arg(s2,8,16,QChar('0')).arg(s3,8,16,QChar('0')).arg(s4,8,16,QChar('0')).toUpper();
    return cpu;
}

QString CMonitor::GetDriveSerial()
{
    //return "";
    QString sSerial;
    QString sMac;
    if(!GetMacAddress(sMac))
    {
        qDebug() << "获取MAC地址失败！";
        return sSerial;
    }
    QString sCpu,sBoardID;
    sCpu = GetCpuID();
    sBoardID = GetBoardID();
    qDebug() << "CPU Serial:" << sCpu;
    qDebug() << "BoardID Serial:" << sBoardID;
    qDebug() << "MAC Address:" << sMac;
    sBoardID = (sBoardID.isEmpty() ? "BOARDID":sBoardID);
    sCpu = (sCpu.isEmpty() ? "CPUID":sCpu);
    sSerial = sBoardID + sCpu + sMac + sCpu + sMac + sBoardID;

    sSerial = QCryptographicHash::hash( sSerial.toLocal8Bit() , QCryptographicHash::Md5 ).toHex().toUpper().data();
    if(sSerial.size() == 32)
    {
        int nTotal = 0;
        sSerial[31] = QChar('0');
        foreach(QChar c , sSerial)
        {
            nTotal += (int)c.toUpper().toLatin1();
        }
        nTotal %= 36;
        char cAppend = '0';
        if(nTotal >= 10)
        {
            nTotal -= 10;
            cAppend = 'A';
        }
        cAppend += nTotal;
        sSerial[31] = QChar(cAppend);
    }

    return sSerial;
}

bool CMonitor::checkSerial(QString sSerial)
{
    if(sSerial.size() == 32)
    {
        int nTotal = 0;
        QChar cCheckBit = sSerial[31];
        sSerial[31] = QChar('0');
        foreach(QChar c , sSerial)
        {
            nTotal += (int)c.toUpper().toLatin1();
        }
        nTotal %= 36;
        char cAppend = '0';
        if(nTotal >= 10)
        {
            nTotal -= 10;
            cAppend = 'A';
        }
        cAppend += nTotal;
        if(cCheckBit == cAppend)
        {
            return true;
        }
    }
    return false;
}

bool CMonitor::checkSerialAndLicenseFile(QString sLicenseFileName)
{
    return true;
    QString sSerial = GetDriveSerial();
    QFile pFile(sLicenseFileName);
    if(pFile.open(QIODevice::ReadOnly))
    {
        QByteArray bReadBuffer = pFile.readAll();
        if(!bReadBuffer.isEmpty() && sSerial.size() == 32 && checkSerial(sSerial))
        {
            QString sLicenseTmp;
            if(GenerateLicense(sSerial,sLicenseTmp) && sLicenseTmp.toLocal8Bit() == bReadBuffer)
            {
                pFile.close();
                return true;
            }
        }
        pFile.close();
    }
    QString sLicenseTmp;
    GenerateLicense(sSerial,sLicenseTmp);
    qDebug() << sLicenseTmp;
    qDebug() << sLicenseFileName << "The license is illegal.";
    return false;
}

bool CMonitor::checkSerialAndLicense(QString sSerial,QString sLicense)
{
    if(!sLicense.isEmpty() && sSerial.size() == 32 && checkSerial(sSerial))
    {
        QString sLicenseTmp;
        if(GenerateLicense(sSerial,sLicenseTmp) && sLicenseTmp == sLicense)
        {
            return true;
        }
    }
    return false;
}

bool CMonitor::GenerateLicense(QString sSerial,QString& sLicense)
{
    sLicense.clear();
    if(sSerial.size() == 32 && checkSerial(sSerial))
    {
        sLicense = QCryptographicHash::hash(sSerial.toLocal8Bit(),QCryptographicHash::Sha3_512).toHex().toUpper().data();
        sLicense += "\r\n";
        sLicense += QCryptographicHash::hash(sLicense.toLocal8Bit(),QCryptographicHash::Sha3_512).toHex().toUpper().data();
        sLicense += "\r\n";
        sLicense += QCryptographicHash::hash(sLicense.toLocal8Bit(),QCryptographicHash::Sha3_512).toHex().toUpper().data();
        sLicense += "\r\n";
        sLicense += QCryptographicHash::hash(sLicense.toLocal8Bit(),QCryptographicHash::Sha3_512).toHex().toUpper().data();
        sLicense += "\r\n";
        return true;
    }
    return false;
}

bool CMonitor::GetProcessPID( QString strProcessName, QVector<quint32> &vtrProcessIDs, bool bSameName )
{
    QString cmd = QString("pidof \"%1\"").arg(strProcessName);
    QByteArray sShellResult;
    FILE *fp = NULL;
    fp = popen(cmd.toStdString().c_str(),"r");
    if(fp)
    {
        char buf[256] = {0};
        int read = 0;
        while((read = fread(buf, sizeof(char),256, fp) ) != 0)
        {
            sShellResult.append(buf,read);
        }
        pclose(fp);
    }
    char* data =sShellResult.data();
    char* ptr = data;
    long pid = 0;
    do
    {
        pid = strtol(ptr,&ptr,10);
        if(pid > 0)
            vtrProcessIDs.push_back(pid);
    }while(pid > 0 && ++ptr < data+sShellResult.size() );

    return true;
}

//double CMonitor::GetProcessCpuUsed(quint32 PID)
//{
//    double nCpuUsed = 1;
//#ifdef Q_OS_LINUX
//    QString cmd = QString("cat /proc/%1/stat").arg(PID);

//    QByteArray sShellResult;
//    FILE *fp = NULL;
//    fp = popen(cmd.toStdString().c_str(),"r");
//    if(fp)
//    {
//        char buf[100] = {0};
//        int read = 0;
//        while((read = fread(buf, sizeof(char),100, fp) ) != 0)
//        {
//            sShellResult.append(buf,read);
//        }
//        pclose(fp);

//        long long ProgressCpu = 0;
//        long long TotalCpu = 0;
//        QStringList sList = QString(sShellResult).split(" ", QString::SkipEmptyParts);
//        for(int i=13; i<17; i++)
//            ProgressCpu += sList[i].toLongLong();
//        GetCpuUsed();
//        if(m_nOldCpuToalTime != 0 && m_nOldProgressCpuTime != 0)
//        {
//            nCpuUsed = (( ProgressCpu - m_nOldProgressCpuTime)/(double)(TotalCpu - m_nOldCpuToalTime)) * 100;
//        }
//        m_nOldProgressCpuTime = ProgressCpu;
//        m_nOldCpuToalTime = TotalCpu;

//    }
//#elif defined(Q_OS_WIN)

//#endif
//    return nCpuUsed;
//}

QString CMonitor::GetProcessUser(quint32 PID)
{
    QString strUser = "";
#ifdef Q_OS_LINUX
    QString cmd = QString("ps aux |grep \" %1 \" |grep -v grep").arg(PID);

    QByteArray sShellResult;
    FILE *fp = NULL;
    fp = popen(cmd.toStdString().c_str(),"r");
    if(fp)
    {
        char buf[1024] = {0};
        int read = 0;
        while((read = fread(buf, sizeof(char),1024, fp) ) != 0)
        {
            sShellResult.append(buf,read);
        }
        pclose(fp);
        QStringList sList = QString(sShellResult).split(QRegExp("\\s+"), QString::SkipEmptyParts);
        if(!sList.isEmpty())
        {
            strUser = sList[0];
        }
    }
#elif defined(Q_OS_WIN)

#endif
    return strUser;
}

double CMonitor::GetProcessCpuUsed(quint32 PID)
{
    double nCpuUsed = 1;
#ifdef Q_OS_LINUX
    QString cmd = QString("ps aux |grep \" %1 \"|grep -v grep").arg(PID);

    QByteArray sShellResult;
    FILE *fp = NULL;
    fp = popen(cmd.toStdString().c_str(),"r");
    if(fp)
    {
        char buf[1024] = {0};
        int read = 0;
        while((read = fread(buf, sizeof(char),1024, fp) ) != 0)
        {
            sShellResult.append(buf,read);
        }
        pclose(fp);
        QStringList sList = QString(sShellResult).split(QRegExp("\\s+"), QString::SkipEmptyParts);
        if(!sList.isEmpty())
        {
            nCpuUsed = sList[2].toDouble();
        }
    }
#elif defined(Q_OS_WIN)

#endif
    return nCpuUsed;
}

quint32 CMonitor::GetProcessMemSize(quint32 PID)
{
    quint32 memSize = 1;
#ifdef Q_OS_LINUX
    QString cmd = QString("cat /proc/%1/status |grep VmRSS").arg(PID);

    QByteArray sShellResult;
    FILE *fp = NULL;
    fp = popen(cmd.toStdString().c_str(),"r");
    if(fp)
    {
        char buf[100] = {0};
        int read = 0;
        while((read = fread(buf, sizeof(char),100, fp) ) != 0)
        {
            sShellResult.append(buf,read);
        }
        pclose(fp);
        QStringList sList = QString(sShellResult).split(QRegExp("\\s+"), QString::SkipEmptyParts);
        if(!sList.isEmpty())
        memSize = sList[1].toDouble();

    }
#elif defined(Q_OS_WIN)

#endif
    return memSize;

}


double CMonitor::GetProcessMemUsed(quint32 PID)
{
    double nMemUsed = 1;
#ifdef Q_OS_LINUX
    QString cmd = QString("cat /proc/meminfo |grep MemTotal");

    QByteArray sShellResult;
    FILE *fp = NULL;
    fp = popen(cmd.toStdString().c_str(),"r");
    if(fp)
    {
        char buf[100] = {0};
        int read = 0;
        while((read = fread(buf, sizeof(char),100, fp) ) != 0)
        {
            sShellResult.append(buf,read);
        }
        pclose(fp);

        QStringList sList = QString(sShellResult).split(QRegExp("\\s+"), QString::SkipEmptyParts);
        nMemUsed = (double)GetProcessMemSize(PID)*100/sList[1].toDouble();
    }
#elif defined(Q_OS_WIN)

#endif
    return nMemUsed;
}

QString CMonitor::GetProcessCommondPath(quint32 PID)
{
    QString strCommond = "";
#ifdef Q_OS_LINUX

    QString cmd = QString("ps aux |grep \" %1 \" |grep -v grep").arg(PID);

    QByteArray sShellResult;
    FILE *fp = NULL;
    fp = popen(cmd.toStdString().c_str(),"r");
    if(fp)
    {
        char buf[1024] = {0};
        int read = 0;
        while((read = fread(buf, sizeof(char),1024, fp) ) != 0)
        {
            sShellResult.append(buf,read);
        }
        pclose(fp);

        QStringList sList = QString(sShellResult).split(QRegExp("\\s+"), QString::SkipEmptyParts);
        if(!sList.isEmpty())
        {
            for(int i = 10; i < sList.count(); i++)
                strCommond = strCommond + QString(" ") + sList[i];
        }
    }
#elif defined(Q_OS_WIN)

#endif
    return strCommond;
}

quint32 CMonitor::GetProcessThreadsNum(quint32 PID)
{
    quint32 ThreadsNum = 1;
#ifdef Q_OS_LINUX
    QString cmd = QString("cat /proc/%1/status |grep Threads").arg(PID);

    QByteArray sShellResult;
    FILE *fp = NULL;
    fp = popen(cmd.toStdString().c_str(),"r");
    if(fp)
    {
        char buf[100] = {0};
        int read = 0;
        while((read = fread(buf, sizeof(char),100, fp) ) != 0)
        {
            sShellResult.append(buf,read);
        }
        pclose(fp);
        QStringList sList = QString(sShellResult).split(QRegExp("\\s+"));
        ThreadsNum = sList[1].toDouble();
    }
#elif defined(Q_OS_WIN)

#endif
    return ThreadsNum;
}

struDiskIO CMonitor::GetProcessDiskIO(quint32 PID)
{
    struDiskIO DiskIO = {0, 0, 0};
#ifdef Q_OS_LINUX
    QString cmd = QString("iotop -p %1 -b -n 1 |grep %1").arg(PID);
    QByteArray sShellResult;
    FILE *fp = NULL;
    fp = popen(cmd.toStdString().c_str(),"r");
    if(fp)
    {
        char buf[200] = {0};
        int read = 0;
        while((read = fread(buf, sizeof(char),200, fp) ) != 0)
        {
            sShellResult.append(buf,read);
        }
        pclose(fp);
        QStringList sList = QString(sShellResult).split(QRegExp("\\s+"));
        DiskIO.diskRead = QString(sList[4]).toDouble();
        DiskIO.diskWrite = QString(sList[6]).toDouble();
        DiskIO.diskIO = QString(sList[10]).toDouble();
    }
#elif defined(Q_OS_WIN)

#endif
    return DiskIO;
}

quint32 CMonitor::GetProcessConnectNum(quint32 PID)
{
    quint32 connectNum = 0;
#ifdef Q_OS_LINUX
    QString cmd = QString("ls /proc/%1/fd -l | grep socket: | wc -l ").arg(PID);

    QByteArray sShellResult;
    FILE *fp = NULL;
    fp = popen(cmd.toStdString().c_str(),"r");
    if(fp)
    {
        char buf[100] = {0};
        int read = 0;
        while((read = fread(buf, sizeof(char),100, fp) ) != 0)
        {
            sShellResult.append(buf,read);
        }
        pclose(fp);
        connectNum = QString(sShellResult).toUInt();
    }
#elif defined(Q_OS_WIN)

#endif
    return connectNum;

}


struNetUsed CMonitor::GetProcessNetUsed(quint32 PID)
{
    struNetUsed netUsed;
    netUsed.netSend = 0;
    netUsed.netRecv = 0;
#ifdef Q_OS_LINUX
    QString cmd = QString("/root/demo/nethogs/src/nethogs -t |grep \" %1 \"").arg(PID);
    QByteArray sShellResult;
    FILE *fp = NULL;
    fp = popen(cmd.toStdString().c_str(),"r");
    QThread::sleep(1);
    popen("pgrep nethogs | xargs kill -s 9","r");
    if(fp)
    {
        char buf[1024] = {0};
        int read = 0;
        while((read = fread(buf, sizeof(char),1024, fp) ) != 0)
        {
            sShellResult.append(buf,read);
        }
        pclose(fp);

        QStringList sList;
        sList = QString(sShellResult).split(QRegExp("\\s+"));
        if(sList.count() != 1 )
        {
            netUsed.netSend = sList[1].toDouble();
            netUsed.netRecv = sList[2].toDouble();
        }
    }
#elif defined(Q_OS_WIN)

#endif
    return netUsed;
}
