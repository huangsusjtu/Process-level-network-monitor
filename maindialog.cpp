#include "maindialog.h"
#include <QPixmap>
#include <QString>
#include <QVBoxLayout>
#include <QLabel>
#include <QWidget>
#include "packet_cap.h"

maindialog::maindialog()
{
	this->dialog = new QDialog;
	dialog->setStyleSheet("maindialog{border-image:url(image/1.jpg)}");

    dialog->setWindowOpacity(0.9);

    verticalLayoutWidget = new QWidget(this->dialog);
    verticalLayoutWidget->setGeometry(QRect(0, 0, 760, 360));

    top = new QVBoxLayout(verticalLayoutWidget);
    label = new QLabel("hehe");
    table = new Qtable;
    table->setAttribute(Qt::WA_TranslucentBackground, true);
    top->addWidget(table);
    top->addWidget(label);
    top->setContentsMargins(0, 0, 0, 0);
    dialog->setLayout(top);
    dialog->setWindowIcon(*new QIcon("image/1.png"));
    dialog->setWindowTitle("PLNM-Process-level network monitor");
    //dialog->setFixedSize(500,385);
}

maindialog::~maindialog()
{
	
}

void maindialog::show()
{
	dialog->show();
}

void maindialog::reflush(const vector<struct process*>& Pprocess)
{
	double tin =((double)process_manager::GetInstance().lenin)/1024;
	double tout = ((double)process_manager::GetInstance().lenout)/1024;
	label->setText("download:"+QString::number(tin)+"kb/s" \
			     +"  upload:"+QString::number(tout)+"kb/s" );

    table->clearContents();
	table->setRowCount(Pprocess.size());
    table->setColumnCount(4);
	
	
   for(unsigned int i=0;i<Pprocess.size();i++)
   {
       table->setItem(i,0,new QTableWidgetItem( *new QString(Pprocess[i]->name ) )); ////插入进程名
	   table->setItem(i,1,new QTableWidgetItem(QString::number(Pprocess[i]->pid) )); ////插入进程名
       table->setItem(i,2,new QTableWidgetItem( QString::number( (double)(Pprocess[i]->len_out)/1024 )+"kb/s" )); //上传
       table->setItem(i,3,new QTableWidgetItem( QString::number( (double)(Pprocess[i]->len_in)/1024 )+"kb/s" ));  //下载
   }
	
}




