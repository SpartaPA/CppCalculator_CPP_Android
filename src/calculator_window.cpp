#include "calculator_window.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QMessageBox>

static const QString ink="#3B3350", soft="#8A81A0", seq="#CFE7FF", setc="#FFD3E8",
                     mat="#D3F5D0", num="#FFF6DD", op="#FFDCC7", eq="#D9C9FF";

CalculatorWindow::CalculatorWindow(QWidget* parent):QMainWindow(parent){
    setWindowTitle("Cpp Calculator");
    setMinimumSize(360,720);
    setStyleSheet("QMainWindow{background:#F3E9FF;} QLabel{color:"+ink+";}");

    auto* root=new QWidget; setCentralWidget(root);
    auto* v=new QVBoxLayout(root); v->setContentsMargins(16,32,16,40); v->setSpacing(0);

    auto* title=new QLabel("Cpp Calculator"); title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet("font-size:30px;font-weight:700;color:"+ink+";");
    auto* desc=new QLabel("수열 ( , ) · 집합 { , } · 행렬 [ , ] 을 버튼으로만 계산합니다\n같은 타입끼리 연산하면 결과도 같은 타입으로 나옵니다");
    desc->setAlignment(Qt::AlignCenter); desc->setStyleSheet("font-size:13px;color:"+soft+";");

    v->addWidget(title);v->addWidget(desc);

    auto* legend=new QHBoxLayout;legend->setSpacing(10);legend->setContentsMargins(0,12,0,18);
    for(auto p:QStringList{"( ) 수열·묶음","{ } 집합","[ ] 행렬"}){
        auto* l=new QLabel(p);QString bg=p.startsWith("(")?seq:(p.startsWith("{")?setc:mat);
        l->setStyleSheet("background:"+bg+";border-radius:14px;padding:4px 9px;font-size:11px;font-weight:700;color:"+ink+";");
        legend->addWidget(l,0,Qt::AlignCenter);
    }
    v->addLayout(legend);

    auto* display=new QFrame;display->setStyleSheet("QFrame{background:#F6FFFA;border:1px solid #E4E0F5;border-radius:16px;}");
    auto* dv=new QVBoxLayout(display);dv->setContentsMargins(16,12,16,12);dv->setSpacing(8);
    auto* cap=new QLabel("CALCULATION AREA");cap->setStyleSheet("font-size:10px;font-weight:700;color:"+soft+";");
    expr=new QLabel;expr->setMinimumHeight(32);expr->setStyleSheet("font-size:22px;color:"+ink+";font-family:monospace;");
    auto* line=new QFrame;line->setFrameShape(QFrame::HLine);line->setStyleSheet("color:#DCD6EE;");
    auto* row=new QHBoxLayout;auto* rc=new QLabel("RESULT OUTPUT");rc->setStyleSheet("font-size:10px;color:"+soft+";");
    result=new QLabel("0");result->setAlignment(Qt::AlignRight);result->setStyleSheet("font-size:19px;font-weight:700;color:#4B2E99;");
    row->addStretch();row->addWidget(rc);row->addSpacing(8);row->addWidget(result);
    dv->addWidget(cap);dv->addWidget(expr);dv->addWidget(line);dv->addLayout(row);
    v->addWidget(display);v->addSpacing(14);

    auto* clear=new QPushButton("전체 지우기 (C)");clear->setStyleSheet("QPushButton{border:none;background:transparent;color:"+soft+";font-size:12px;}");clear->setCursor(Qt::PointingHandCursor);
    connect(clear,&QPushButton::clicked,this,&CalculatorWindow::clearAll);v->addWidget(clear,0,Qt::AlignRight);v->addSpacing(4);

    auto* grid=new QGridLayout;grid->setHorizontalSpacing(9);grid->setVerticalSpacing(9);
    const QString keys[6][4]={{"(","{","[","⌫"},{")","}","]","÷"},{"7","8","9","×"},{"4","5","6","−"},{"1","2","3","+"},{"0",".",",","="}};
    for(int r=0;r<6;++r)for(int c=0;c<4;++c){auto* b=makeButton(keys[r][c],QString());grid->addWidget(b,r,c);}
    v->addLayout(grid);v->addSpacing(16);

    auto* foot=new QLabel("( a, b ) 수열 · { a, b } 집합 · [ [a,b],[c,d] ] 행렬 · 원소 1개짜리 ( )는 그냥 묶음 괄호로 처리됩니다 · 별(*) 두 번 = 제곱(**) · 슬래시(/)는 역행렬 곱 · 입력은 버튼으로만 가능합니다");
    foot->setWordWrap(true);foot->setAlignment(Qt::AlignCenter);foot->setStyleSheet("font-size:11px;color:"+soft+";");
    v->addWidget(foot);
    render();
}

QPushButton* CalculatorWindow::makeButton(const QString& k,const QString&){
    auto* b=new QPushButton(k);b->setMinimumHeight(58);b->setStyleSheet(
        "QPushButton{border:none;border-radius:14px;background:"+(
        k=="("||k==")"?seq:k=="{"||k=="}"?setc:k=="["||k=="]"?mat:k=="⌫"?"#F0E9FB":k=="÷"||k=="×"||k=="−"||k=="+"?op:k=="="?eq:num)+
        ";color:"+ink+";font-size:18px;font-weight:700;}QPushButton:pressed{margin:2px;}");
    connect(b,&QPushButton::clicked,this,[this,k]{click(k);});return b;
}
void CalculatorWindow::click(const QString& k){
    if(k=="⌫"){if(cursor>0){text.remove(cursor-1,1);--cursor;}render();return;}
    if(k=="="){
        try{std::string s=text.toStdString();result->setText(QString::fromStdString(calc::CalculatorEngine::format(engine.calc(s))));result->setStyleSheet("font-size:19px;font-weight:700;color:#4B2E99;");}
        catch(const std::exception&e){result->setText("오류: "+QString::fromUtf8(e.what()));result->setStyleSheet("font-size:19px;font-weight:700;color:#E5537A;");}
        return;
    }
    QString x=k;x.replace("÷","/").replace("×","*").replace("−","-");
    text.insert(cursor,x);cursor+=x.size();render();
}
void CalculatorWindow::clearAll(){text.clear();cursor=0;result->setText("0");result->setStyleSheet("font-size:19px;font-weight:700;color:#4B2E99;");render();}
void CalculatorWindow::render(){expr->setText((text.isEmpty()?"여기에 버튼으로 입력하세요":text)+" |");}
