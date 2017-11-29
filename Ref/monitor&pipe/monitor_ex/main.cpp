#include <QCoreApplication>
#include "Monitor.h"
#include <QDebug>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    CMonitor objMonitor;
//    QString strMacAddr = "";
//    objMonitor.GetMacAddress(strMacAddr);

//    qDebug() << "BoardID:" << objMonitor.GetBoardID();
//    qDebug() << "CpuID:" << objMonitor.GetCpuID();
//    qDebug() << "MacAddr:" << strMacAddr;


//    qDebug() << "DriveSerial:" << objMonitor.GetDriveSerial();


//    qDebug() << "resourceUsed====================================";
//    qDebug() << "CpuUsed:" << objMonitor.GetCpuUsed();
//    qDebug() << "DiskUsed:" << objMonitor.GetDiskUsed();
//    qDebug() << "MemUsed:" << objMonitor.GetMemUsed();
//    qDebug() << "NetUsed:" << objMonitor.GetNetUsed();
    QVector<quint32> vtrProcessIDs;

    objMonitor.GetProcessPID("firefox",vtrProcessIDs,true);
    for (int i = 0; i < vtrProcessIDs.count();i++)
    {
        qDebug() << "ProcessID:" << vtrProcessIDs[i];
        qDebug() << "ProcessUser:" << objMonitor.GetProcessUser(vtrProcessIDs[i]);
        qDebug() << "ProcessCpuUsed:" << objMonitor.GetProcessCpuUsed(vtrProcessIDs[i]);
        qDebug() << "ProcessMemUsedSize:" << objMonitor.GetProcessMemSize(vtrProcessIDs[i]) << "kb";
        qDebug() << "ProcessMemUsed:" << objMonitor.GetProcessMemUsed(vtrProcessIDs[i]) << "%";
        qDebug() << "ProcessPath:" << objMonitor.GetProcessCommondPath(vtrProcessIDs[i]);
        qDebug() << "ThreadsNum:" << objMonitor.GetProcessThreadsNum(vtrProcessIDs[i]);
        struDiskIO disk = objMonitor.GetProcessDiskIO(vtrProcessIDs[i]);
        qDebug() << "diskRead:" << disk.diskRead << "B/s";
        qDebug() << "diskWrite:" << disk.diskWrite << "B/s";
        qDebug() << "diskIO:" << disk.diskIO << "B/s";
        qDebug() << "ProcessConnectNum:" << objMonitor.GetProcessConnectNum(vtrProcessIDs[i]);
        struNetUsed net = objMonitor.GetProcessNetUsed(vtrProcessIDs[i]);
        qDebug() << "send:" << net.netSend << "kb/s";
        qDebug() << "recieve:" << net.netRecv << "kb/s";
    }
    return a.exec();
}
