#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QMessageBox>
#include <QDebug>
#include <QFileDialog>
#include <QStandardPaths>

#include <cmath>
#include <fstream>

using QMsg = QMessageBox;
#define cout qDebug()

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->path->setText(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation));
    ui->d_in->setValue(45.0);
}

MainWindow::~MainWindow()
{
    delete ui;
}

std::list<real> MainWindow::calculate(const real ub, real d_in, real d_out) const noexcept
{
    if(d_in == -1.0) d_in = ui->d_in->value();
    if(d_out == -1.0) d_out = ui->d_out->value();
    const real op_ub = 1.0 - ub;
    std::list<real> res;
    while(true)
    {
        if(d_in * op_ub < d_out)
        {
            res.push_back(((d_in - d_out)/d_in));
            return res;
        }
        else
        {
            res.push_back(ub);
            d_in = d_in * op_ub;
        }
    }
}

void MainWindow::gen_xlsx(const bool ext) const
{
    const QString sheet_1{ "MINIMALNA ILOŚĆ CIĄGÓW" };
    const QString sheet_2{ "MAKSYMALNA ILOŚĆ CIĄGÓW" };

    Document doc;

    //Sheets
    doc.addSheet(sheet_1);
    doc.addSheet(sheet_2);
    doc.selectSheet(sheet_1);

    //Data to save and lambdas
    std::list<real> min = calculate(ui->ub_max->value()/100.0);   //Sheet 1
    std::list<real> max = calculate(ui->ub_min->value()/100.0);   //Sheet 2
    const real d_in = ui->d_in->value();
    real d = d_in;
    const real d_out = ui->d_out->value();
    const real V = d_in * input_height;
    auto get_height = [&](const real _d)
    {
        return V / ( M_PI * ( ( _d * _d ) / 4.0 ) );
    };

    //Format
    Format base;
        base.setVerticalAlignment(Format::VerticalAlignment::AlignVCenter);
        base.setHorizontalAlignment(Format::HorizontalAlignment::AlignHCenter);
    Format head{ base };
        head.setFontBold(true);
    Format tekst{ base };
        tekst.setNumberFormat("@");
    Format nums{ base };
        nums.setNumberFormat("#.00\" mm\"");
    Format LP{ base };
        nums.setNumberFormat("#.00");
    Format percentage{ base };
        percentage.setNumberFormat("#.00 %");

    //Header
    doc.write(1, 1, "LP", head);
    doc.write(1, 2, "d_i [mm]", head);
    doc.write(1, 3, "q_i [%]", head);
    doc.write(1, 4, "q_c [%]", head);
    doc.write(1, 5, "wydł_całkowite [mm]", head);
    doc.write(1, 6, "wydł_wzgledne [mm]", head);

    #define n2v(val) QVariant(static_cast<uint>(val))


    //Data MIN POPRAW INDEKSY
    int cnt = 2;
    doc.write( cnt, 1, n2v(0), LP );
    doc.write( cnt, 2, d_in, nums );
    doc.write( cnt, 3, "-", tekst );
    doc.write( cnt, 4, "-", tekst );
    doc.write( cnt, 5, "-", tekst );
    doc.write( cnt, 6, "-", tekst );

    real prev_d = d_in;
    // cout << "MINIMUM: " << min.size() << d_in << d_out << ui->ub_max->value() << ui->ub_min->value();
    for( const real var : min )
    {
        cnt++;
        d *= (1.0 - var);
        doc.write(cnt, 1, n2v(cnt - 2), LP);
        doc.write(cnt, 2, d, nums);
        doc.write(cnt, 3, var, percentage);
        doc.write(cnt, 4, 1.0 - (d/d_in), percentage);
        doc.write(cnt, 5, get_height(d) - get_height(d_in), nums);
        doc.write(cnt, 6, get_height(d) - get_height(prev_d), nums);
        // cout << cnt -1 << d << var << (d/d_in) << get_height(d) - get_height(d_in) << get_height(d) - get_height(prev_d);
        prev_d = d;
    }

//###################################################################################

    //Data MAX
    doc.selectSheet(sheet_2);

    doc.write(1, 1, "LP", head);
    doc.write(1, 2, "d_i [mm]", head);
    doc.write(1, 3, "q_i [%]", head);
    doc.write(1, 4, "q_c [%]", head);
    doc.write(1, 5, "wydł_całkowite [mm]", head);
    doc.write(1, 6, "wydł_wzgledne [mm]", head);

    cnt = 2;
    doc.write( cnt, 1, n2v(0), LP );
    doc.write( cnt, 2, d_in, nums );
    doc.write( cnt, 3, "-", tekst );
    doc.write( cnt, 4, "-", tekst );
    doc.write( cnt, 5, "-", tekst );
    doc.write( cnt, 6, "-", tekst );

    prev_d = d_in;
    d = d_in;
    // cout << "MAXIMUM: ";
    for( const real var : max )
    {
        cnt++;
        d *= (1.0 - var);
        doc.write(cnt, 1, n2v(cnt - 2), LP);
        doc.write(cnt, 2, d, nums);
        doc.write(cnt, 3, var, percentage);
        doc.write(cnt, 4, 1.0 - (d/d_in), percentage);
        doc.write(cnt, 5, get_height(d) - get_height(d_in), nums);
        doc.write(cnt, 6, get_height(d) - get_height(prev_d), nums);
        // cout << cnt -1 << d << var << (d/d_in) << get_height(d_in) - get_height(d) << get_height(prev_d) - get_height(d);
        prev_d = d;
    }

    if(ext)
    {
        std::string D_IN{std::to_string(d_in).c_str()};
        std::string D_OUT{std::to_string(d_out).c_str()};
        const int steps = ui->ciagi->value();
        std::string CUST{std::to_string(steps).c_str()};
        const double q_c = ((d_in-d_out)/d_in) * 100.0;
        const double l_c = std::pow(d_in/d_out, 2.0);
        const double l_i = std::pow(l_c, 1.0/static_cast<double>(steps));
        const double q_i = 1.0 - std::sqrt(1.0 - (1.0 - (1/l_i)));
        std::list<real> cust = calculate(q_i);
        doc.addSheet("WYBRANA ILOŚĆ CIĄGÓW");

        doc.write(1, 1, "LP", head);
        doc.write(1, 2, "d_i [mm]", head);
        doc.write(1, 3, "q_i [%]", head);
        doc.write(1, 4, "q_c [%]", head);
        doc.write(1, 5, "wydł_całkowite [mm]", head);
        doc.write(1, 6, "wydł_wzgledne [mm]", head);

        cnt = 2;
        doc.write( cnt, 1, n2v(0), LP );
        doc.write( cnt, 2, d_in, nums );
        doc.write( cnt, 3, "-", tekst );
        doc.write( cnt, 4, "-", tekst );
        doc.write( cnt, 5, "-", tekst );
        doc.write( cnt, 6, "-", tekst );

        prev_d = d_in;
        d = d_in;
        // cout << "CUST: ";
        for( const real var : cust )
        {
            cnt++;
            if(cnt - 2 == steps + 1) break;
            d *= (1.0 - var);
            doc.write(cnt, 1, n2v(cnt - 2), LP);
            doc.write(cnt, 2, d, nums);
            doc.write(cnt, 3, var, percentage);
            doc.write(cnt, 4, 1.0 - (d/d_in), percentage);
            doc.write(cnt, 5, get_height(d) - get_height(d_in), nums);
            doc.write(cnt, 6, get_height(d) - get_height(prev_d), nums);
            // cout << cnt -1 << d << var << (d/d_in) << get_height(d_in) - get_height(d) << get_height(prev_d) - get_height(d);
            prev_d = d;
        }
    }
    doc.selectSheet(sheet_1);
    doc.saveAs(QDir::toNativeSeparators( ui->path->text() + "/" + ui->filename->text() ));
    QMsg a{QMsg::NoIcon, "Informacja", "Wygenerowano pomylnie"};
    a.exec();
}

void MainWindow::on_d_in_valueChanged(double arg1)
{
    if(arg1 <= 0.0 || arg1 < ui->d_out->value())
    {
        QMsg a{QMsg::Icon::NoIcon, "Błąd Danych", "Wprowadzona wartość musi być większe od 0 oraz od średnicy wyjściowej"};
        a.show();
        a.exec();
        ui->d_in->setValue(ui->d_out->value() + 1.0);
    }
    else
    {
        std::list<real> res_max{ calculate(ui->ub_min->value()/100.0) };
        std::list<real> res_min{ calculate(ui->ub_max->value()/100.0) };
        ui->ciagi->setRange(res_min.size(), res_max.size());
        ui->ciag_max->setText(QString::number(res_max.size()));
        ui->ciag_min->setText(QString::number(res_min.size()));
    }
}

void MainWindow::on_d_out_valueChanged(double arg1)
{
    if(arg1 <= 0.0  || arg1 > ui->d_in->value())
    {
        QMsg a{QMsg::Icon::NoIcon, "Błąd Danych", "Wprowadzona wartość musi być większe od 0 i być mniejsze od średnicy wejściowej"};
        a.show();
        a.exec();
        ui->d_out->setValue(ui->d_in->value() - 0.1);
    }
    else
    {
        std::list<real> res_max{ calculate(ui->ub_min->value()/100.0) };
        std::list<real> res_min{ calculate(ui->ub_max->value()/100.0) };
        ui->ciagi->setRange(res_min.size(), res_max.size());
        ui->ciag_max->setText(QString::number(res_max.size()));
        ui->ciag_min->setText(QString::number(res_min.size()));
    }
}

void MainWindow::on_ub_min_valueChanged(double arg1)
{
    if(arg1 <= 0.0 || arg1 > 100.0)
    {
        QMsg a{QMsg::Icon::NoIcon, "Błąd Danych", "Wprowadzona wartość musi być większe od 0 i mniejsze od 100"};
        a.show();
        a.exec();
        ui->d_in->setValue(1.0);
    }
    else
    {
        std::list<real> res_max{ calculate(ui->ub_min->value()/100.0) };
        std::list<real> res_min{ calculate(ui->ub_max->value()/100.0) };
        ui->ciagi->setRange(res_min.size(), res_max.size());
        ui->ciag_max->setText(QString::number(res_max.size()));
        ui->ciag_min->setText(QString::number(res_min.size()));
    }
}

void MainWindow::on_ub_max_valueChanged(double arg1)
{
    if(arg1 <= 0.0 || arg1 > 100.0)
    {
        QMsg a{QMsg::Icon::NoIcon, "Błąd Danych", "Wprowadzona wartość musi być większe od 0 i mniejsze od 100"};
        a.show();
        a.exec();
        ui->d_in->setValue(1.0);
    }
    else
    {
        std::list<real> res_max{ calculate(ui->ub_min->value()/100.0) };
        std::list<real> res_min{ calculate(ui->ub_max->value()/100.0) };
        ui->ciagi->setRange(res_min.size(), res_max.size());
        ui->ciag_max->setText(QString::number(res_max.size()));
        ui->ciag_min->setText(QString::number(res_min.size()));
    }
}

void MainWindow::on_ciagi_valueChanged(int value)
{
    ui->ciag_val->setText(QString::number(value));
}

void MainWindow::on_select_path_clicked()
{
    ui->path->setText( QFileDialog::getExistingDirectory( this, "Wybierz miejsce zapisu", QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) ) );
}

void MainWindow::on_pushButton_clicked()
{
    if(ui->path->text() != QString() && ui->filename->text() != QString())
    {
        gen_xlsx(true);
    }
    else
    {
        QMsg a{ QMsg::NoIcon, "Błąd", "Podano niepoprawną ścieżkę lub nazwę pliku" };
        a.exec();
    }
}

void MainWindow::on_pushButton_2_clicked()
{
    if(ui->path->text() != QString() && ui->filename->text() != QString())
    {
        gen_xlsx();
    }
    else
    {
        QMsg a{ QMsg::NoIcon, "Błąd", "Podano niepoprawną ścieżkę lub nazwę pliku" };
        a.exec();
    }
}
