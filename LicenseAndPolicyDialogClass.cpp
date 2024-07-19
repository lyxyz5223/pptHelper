#include "LicenseAndPolicyDialogClass.h"

#include <string>
int TimeLeft = 15;
QString AgreeBtnText;
LicenseAndPolicyDialogClass::LicenseAndPolicyDialogClass(QWidget* parent) : QDialog(parent)
{
	ui.setupUi(this);
	AgreeBtnText = ui.AgreeButton->text();
	TimeCountDown1 = new QTimer(this);
	connect(TimeCountDown1, SIGNAL(timeout()), this, SLOT(TimeCountDownProc()));
	TimeCountDown1->start(1000);

}

LicenseAndPolicyDialogClass::~LicenseAndPolicyDialogClass()
{
}

void LicenseAndPolicyDialogClass::TimeCountDownProc()
{
	if (TimeLeft > 0)
	{
		ui.AgreeButton->setText(AgreeBtnText + QString::fromLocal8Bit("(") + QString::fromStdString(std::to_string(TimeLeft)) + QString::fromLocal8Bit("Ãë)"));
		TimeLeft--;

	}
	else {
		ui.AgreeButton->setText(AgreeBtnText);
		ui.AgreeButton->setEnabled(true);
		TimeCountDown1->stop();

	}
		ui.AgreeButton->adjustSize();
}