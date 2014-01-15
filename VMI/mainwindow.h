#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>
#include <QMap>
#include <QList>
#include <QStandardItemModel>
#include <QDebug>
#include <QStandardItem>
#include "qpcap.h"
#include "qpcapethernetpacket.h"
#include "qpcapippacket.h"
#include "qpcaptcppacket.h"
#include <QSocketNotifier>
#include <QTableView>
#include <QAbstractItemView>
#include <QTimer>
#include <QTime>
#include <qcustomplot.h>
extern "C"
{
#include <xenstat.h>
#include "pcap.h"
#include <stdio.h>
#include <string.h>
#include <malloc.h>
}
typedef unsigned long long ull;
typedef ull cpu_ns_t;
typedef QVector<cpu_ns_t> cpu_ns_list_t;
typedef QPair<QTime, cpu_ns_t> cpu_pct_pair_t;
typedef QVector<double> cpu_rate_list_t;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT


public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();


    int sniffer();
    static void process_packet(u_char *self, const struct pcap_pkthdr* pkthdr, const u_char *packet);


public slots:
    int network();
    void network_start();
    void network_stop();
    void get_devices();
    void output_packet(QPcapHeader pheader, const uchar *packet);

    void cpu();
    void cpu_xenstat();
    void cpu_rate_calc();
    void cpu_plot();
    void cpu_stop();
    void cpu_get_dom();


private:
    Ui::MainWindow *ui;

    QStandardItemModel *model;
    QMap<QString, pcap_if_t*> dev_name_map;

    QVector<cpu_pct_pair_t> cpu_info_list;
    QTimer *cpu_timer;
    QCustomPlot *plot;
    QVector<QCustomPlot*> plot_list;

    QPcap *qpcap;

    QSocketNotifier *sk_notifier;

    /* Xen stuffs */

};

#endif // MAINWINDOW_H
