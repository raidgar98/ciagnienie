#pragma once
#include <QMainWindow>
#include "xlsxdocument.h"

using real = double;
using namespace QXlsx;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{

    Q_OBJECT

public:

    const real input_height{ 500.0 }; //[mm]

    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    std::list<real> calculate(const real ub, real d_in = -1.0, real d_out = -1.0) const noexcept;

    void gen_xlsx(const bool ext = false) const;

private slots:

    void on_d_in_valueChanged(double arg1);

    void on_d_out_valueChanged(double arg1);

    void on_ub_min_valueChanged(double arg1);

    void on_ub_max_valueChanged(double arg1);

    void on_ciagi_valueChanged(int value);

    void on_select_path_clicked();

    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

private:

    Ui::MainWindow *ui;
};
