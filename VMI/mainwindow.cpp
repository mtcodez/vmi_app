#include "mainwindow.h"
#include "ui_mainwindow.h"


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    cpu_timer = new QTimer(this);
    qpcap = new QPcap(parent);

    /* Set up all the signals*/
    connect(ui->pushButton_start, SIGNAL(clicked()), this, SLOT(network()));
    connect(ui->pushButton_stop, SIGNAL(clicked()), this, SLOT(network_stop()));
    connect(ui->pushButton_refresh, SIGNAL(clicked()), this, SLOT(get_devices()));
    connect(qpcap, SIGNAL(packetReady(QPcapHeader,const uchar*)), this, SLOT(output_packet(QPcapHeader,const uchar*)));
    connect(ui->pushButton_cpu, SIGNAL(clicked()), this, SLOT(cpu()));
    connect(ui->pushButton_cpu_stop, SIGNAL(clicked()), this, SLOT(cpu_stop()));
    connect(ui->pushButton_refresh_dom_list, SIGNAL(clicked()), this, SLOT(cpu_get_dom()));


    model = new QStandardItemModel();
    ui->tableView->setModel(model);
    model->setHorizontalHeaderItem(0, new QStandardItem(QObject::tr("Time")));
    model->setHorizontalHeaderItem(1, new QStandardItem(QObject::tr("Source")));
    model->setHorizontalHeaderItem(2, new QStandardItem(QObject::tr("Destination")));
    model->setHorizontalHeaderItem(3, new QStandardItem(QObject::tr("Protocol")));
    model->setHorizontalHeaderItem(4, new QStandardItem(QObject::tr("Length")));
    model->setHorizontalHeaderItem(5, new QStandardItem(QObject::tr("Data")));

    ui->tableView->setColumnWidth(0, 60);
    ui->tableView->setColumnWidth(1, 110);
    ui->tableView->setColumnWidth(2, 110);
    ui->tableView->setColumnWidth(3, 80);
    ui->tableView->setColumnWidth(4, 50);
    ui->tableView->horizontalHeader()->setStretchLastSection(true);



}

MainWindow::~MainWindow()
{
    delete ui;
}

int MainWindow::network()
{
    /*
    QString device = qpcap->lookupDevice();
    if ( device.isEmpty() ) {
        qDebug() << "Lookup device failed, " << qpcap->errorString();
        return 1;
    }
*/
    QString device = ui->comboBox->currentText();
    bool ok;
    qDebug() << "device:" << device;
    ok = qpcap->open( device, 65535, true );
    if (!ok) {
        qDebug() << "Unable to open, " << qpcap->errorString();
        return 1;
    }
    qpcap->start();
    return 0;
}

void MainWindow::network_start()
{
    //    qpcap->start();
}

void MainWindow::network_stop()
{
    qpcap->stop();
}

void MainWindow::cpu()
{
    qDebug() << "cpu():";
    // PLOT THING

    uint plot_num = ui->comboBox_dom_name->count();
    for(int i = 0; i < plot_num; ++i)
    {
        // create graph and assign data to it:
        plot_list.append(new QCustomPlot());
        // give the axes some labels:
        plot_list.back()->xAxis->setLabel(QString("Time Domain:%1").arg(i));
        plot_list.back()->yAxis->setLabel("Usage Rate");
        plot_list.back()->xAxis->setTickLabelType(QCPAxis::ltDateTime);
        plot_list.back()->xAxis->setDateTimeFormat("hh:mm:ss");
        plot_list.back()->axisRect()->setupFullAxesBox();
        plot_list.back()->yAxis->setRange(0.0, 120.0);
        plot_list.back()->rescaleAxes(true);
        ui->horizontalLayout_3->addWidget(plot_list.back());
        plot_list.back()->addGraph();
        plot_list.back()->graph()->setPen(QPen(Qt::blue));
        plot_list.back()->graph()->setLineStyle(QCPGraph::lsLine);
        plot_list.back()->graph()->setBrush(QBrush(QColor(255/4.0,160,50,150)));
    }
    /*
    ui->widget_plot->addGraph();
    // give the axes some labels:
    ui->widget_plot->xAxis->setLabel("Time");
    ui->widget_plot->yAxis->setLabel("Usage Rate");

    ui->widget_plot->graph()->setPen(QPen(Qt::blue));
    ui->widget_plot->graph()->setLineStyle(QCPGraph::lsLine);
//    ui->widget_plot->graph()->setScatterStyle(QCPScatterStyle::ssDisc);
    ui->widget_plot->xAxis->setTickLabelType(QCPAxis::ltDateTime);
    ui->widget_plot->xAxis->setDateTimeFormat("hh:mm:ss");
    ui->widget_plot->axisRect()->setupFullAxesBox();
    ui->widget_plot->graph()->setBrush(QBrush(QColor(255/4.0,160,50,150)));
    ui->widget_plot->yAxis->setRange(0.0, 120.0);
    ui->widget_plot->rescaleAxes(true);

//    ui->widget_plot->yAxis->setAutoTicks(false);
//    ui->widget_plot->yAxis->setAutoTickLabels(false);
//    ui->widget_plot->yAxis->setTickVector(QVector<double>() << 30 << 70);
//    ui->widget_plot->yAxis->setTickVectorLabels(QVector<QString>() << "Not so\nhigh" << "Very\nhigh");
*/
    connect(cpu_timer, SIGNAL(timeout()), this, SLOT(cpu_rate_calc()));
    cpu_timer->start(500);


}

void MainWindow::cpu_xenstat()
{
    qDebug() << "cpu_xenstat():";
    // Get node info
    xenstat_domain **domains;
    unsigned int num_domains = 0;
    xenstat_handle *xhandle = NULL;
    xhandle = xenstat_init();
    xenstat_node *cur_node = xenstat_get_node(xhandle, XENSTAT_ALL);

    num_domains = xenstat_node_num_domains(cur_node);
    domains = (xenstat_domain **)calloc(num_domains, sizeof(xenstat_domain *));
    if(domains == NULL)
        fprintf(stderr, "Failed to allocate memory\n");
    uint k;
    for (k=0; k < num_domains; k++)
        domains[k] = xenstat_node_domain_by_index(cur_node, k);
    for (k=0; k < num_domains; k++)
    {
        cpu_ns_t cpu_ns = xenstat_domain_cpu_ns(domains[k]);
        cpu_info_list.append(cpu_pct_pair_t(QTime::currentTime(), cpu_ns));
    }
    xenstat_uninit(xhandle);

}

void MainWindow::cpu_rate_calc()
{
    cpu_xenstat();
    QVector<cpu_pct_pair_t> old_list = cpu_info_list;
    cpu_info_list.clear();
    cpu_xenstat();


    double dom0;
    cpu_rate_list_t rate_list;
    for(int i = 0; i < cpu_info_list.size(); ++i)
    {
        double rate = (double)(cpu_info_list[i].second - old_list[i].second)/(cpu_info_list[i].first.msecsSinceStartOfDay() -old_list[i].first.msecsSinceStartOfDay())/10000;
        rate_list.append(rate);

        double key = QDateTime::currentDateTime().toMSecsSinceEpoch()/1000.0;
        plot_list[i]->graph()->addData(key, rate);
        plot_list[i]->xAxis->setRange(key+0.25, 8, Qt::AlignRight);
        plot_list[i]->graph()->removeDataBefore(key-8);
        plot_list[i]->replot();
//        qDebug() << "CPU " << i << "rate:" << rate << "%";

    }
//    double key = QDateTime::currentDateTime().toMSecsSinceEpoch()/1000.0;
//    ui->widget_plot->graph()->addData(key, dom0);
//    ui->widget_plot->xAxis->setRange(key+0.25, 8, Qt::AlignRight);
//    ui->widget_plot->graph(0)->removeDataBefore(key-8);
//    ui->widget_plot->replot();

}

void MainWindow::cpu_plot()
{
    plot->replot();
}

void MainWindow::cpu_stop()
{
    cpu_timer->stop();
}

void MainWindow::cpu_get_dom()
{
    xenstat_domain **domains;
    unsigned int num_domains = 0;
    xenstat_handle *xhandle = NULL;
    xhandle = xenstat_init();
    xenstat_node *cur_node = xenstat_get_node(xhandle, XENSTAT_ALL);

    num_domains = xenstat_node_num_domains(cur_node);
    domains = (xenstat_domain **)calloc(num_domains, sizeof(xenstat_domain *));
    if(domains == NULL)
        fprintf(stderr, "Failed to allocate memory\n");
    uint k;
    for (k=0; k < num_domains; k++)
    {
        domains[k] = xenstat_node_domain_by_index(cur_node, k);
        ui->comboBox_dom_name->addItem(QString(xenstat_domain_name(domains[k])));
    }

    xenstat_uninit(xhandle);
}

void MainWindow::get_devices()
{
    QStringList list = qpcap->findalldevs();

    int i;
    for(i = 0; i < list.length(); ++i)
    {
        ui->comboBox->addItem(list.at(i));
    }
}

void MainWindow::output_packet(QPcapHeader pheader, const uchar *packet)
{
    QList<QStandardItem *> item_list;

    // Time
    QString time;
    time.setNum(pheader.timeStamp().tv_usec);
    item_list.append(new QStandardItem(time));


    QPcapEthernetPacket ether(packet);

    QPcapIpPacket ip = ether.toIpPacket();
    if (ether.isIpPacket()) {
        // Source
        item_list.append(new QStandardItem(ip.source().toString()));
        // Dest
        item_list.append(new QStandardItem(ip.dest().toString()));
        // Protocol
        item_list.append(new QStandardItem(ip.protocol_str()));
        // TODO show data below IP protocol, i.e. tcp udp etc.
    }
    else
    {
        // Source
        item_list.append(new QStandardItem(ether.sourceHost()));
        item_list.append(new QStandardItem(ether.destHost()));
        item_list.append(new QStandardItem(QString("Ethernet")));
    }

    // Length
    QString len;
    item_list.append(new QStandardItem(len.setNum(pheader.packetLength())));
    // Data
    QByteArray qa(reinterpret_cast<const char*>(packet));
    QLatin1String data(qa.toHex());
    item_list.append(new QStandardItem(data));


    model->appendRow(item_list);
    qDebug() << "output_packet():";
}

