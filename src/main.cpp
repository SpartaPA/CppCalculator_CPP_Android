#include <QApplication>
#include "calculator_window.h"

int main(int argc,char* argv[]){
    QApplication app(argc,argv);
    app.setApplicationName("Cpp Calculator");
    app.setOrganizationName("PA5");
    CalculatorWindow w;
    w.show();
    return app.exec();
}
