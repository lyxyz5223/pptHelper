#pragma once
#include "ui_LicenseAndPolicyDialogClass.h"
#include <qdialog.h>
#include <qtimer.h>


class LicenseAndPolicyDialogClass :
    public QDialog
{
    Q_OBJECT
public:
    LicenseAndPolicyDialogClass(QWidget* parent = nullptr);
    ~LicenseAndPolicyDialogClass();

public slots:
    void TimeCountDownProc();

private:
    Ui::LicenseAndPolicyDialog ui;
    QTimer* TimeCountDown1;

};

