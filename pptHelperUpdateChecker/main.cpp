#include "pptHelperUpdateChecker.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    pptHelperUpdateChecker w;
    w.show();
    return a.exec();
}
