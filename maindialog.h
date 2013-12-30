#ifndef MAINDIALOG_H
#define MAINDIALOG_H
#include <QDialog>
#include <QVBoxLayout>
#include <QLabel>
#include <vector>
using std::vector;
#include <QTableWidget>
#include <QHeaderView>
#include <QList>

#include "process.h"


class Qtable : public QTableWidget
{
	
public:
    Qtable()
	{
    this->setColumnCount(4);
    list <<"Process"<<"PID"<<"Upload"<<"Download";
    this->setHorizontalHeaderLabels(list);
    this->setColumnWidth(3,150);
    this->setFixedSize(760,360);

	}
	
    ~Qtable(){}

public:
    QStringList list;
};

 
class maindialog
{
    
public:
	static class maindialog& GetInstance(){static class maindialog s; return s;}
	maindialog();
	virtual ~maindialog();
    void reflush(const vector<struct process*>& Pprocess);
	void show();
private:
    QWidget *verticalLayoutWidget;
    QLabel *label;
    QVBoxLayout *top;
    Qtable *table;
	QDialog *dialog;
};



#endif // MAINDIALOG_H
