#ifndef MONITOR_H
#define MONITOR_H
#include <QtCore/qglobal.h>
#include <qdatetime.h>
typedef struct {
    double diskRead;
    double diskWrite;
    double diskIO;
}struDiskIO;

typedef struct {
    double netSend;
    double netRecv;
} struNetUsed;

class CMonitor
{
public:
    CMonitor();
    double GetCpuUsed();
    double GetMemUsed();
    double GetDiskUsed();
    double GetNetUsed();

    static QString GetCpuID();
    static QString GetBoardID();
    static QString GetDriveSerial();
    static bool GetMacAddress(QString &strMac);
    static bool checkSerial(QString sSerial);
    static bool checkSerialAndLicense(QString sSerial, QString sLicense);
    static bool GenerateLicense(QString sSerial, QString &sLicense);
    static bool checkSerialAndLicenseFile(QString sLicenseFileName);

    bool GetProcessPID( QString strProcessName, QVector<quint32> &vtrProcessIDs, bool bSameName );
    QString GetProcessUser(quint32 PID);
    double GetProcessCpuUsed(quint32 PID);
    quint32 GetProcessMemSize(quint32 PID);
    double GetProcessMemUsed(quint32 PID);
    QString GetProcessCommondPath(quint32 PID);
    quint32 GetProcessThreadsNum(quint32 PID);
    struDiskIO GetProcessDiskIO(quint32 PID);
    quint32 GetProcessConnectNum(quint32 PID);
    struNetUsed GetProcessNetUsed(quint32 PID);


    static QString m_sEthName;
private:
    QTime m_NetCheck;


    long long m_nOldNetRecv;
    long long m_nOldNetSend;
#ifdef Q_OS_WIN
    double m_nOldCpuToalTime;
    double m_nOldCpuIdleTime;
    QString m_sMacAddress;
#elif defined(Q_OS_LINUX)
    long long m_nOldCpuToalTime;
    long long m_nOldCpuIdleTime;
    long long m_nOldProgressCpuTime;
#endif
};

#endif // MONITOR_H
