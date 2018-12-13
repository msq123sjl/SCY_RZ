#ifndef FRMSAMPLER_H
#define FRMSAMPLER_H

#include <QWidget>

namespace Ui {
class frmSampler;
}

class frmSampler : public QWidget
{
    Q_OBJECT
    
public:
    explicit frmSampler(QWidget *parent = 0);
    ~frmSampler();

   static bool GetSamplerStatus();
   static bool CleanSamplerStatus();
    static bool SampleFixFlow();


    
private slots:
//    void on_comboBox_sampleMode_currentIndexChanged(int index);


    void on_btn_StartSampling_clicked();

    void on_btn_GetSamplerStatus_clicked();

    void on_btn_Cancel_clicked();

    void on_btn_setsave_clicked();

    void on_btn_CleanSamplerStatus_clicked();

private:
    Ui::frmSampler *ui;

    void InitStyle();
    void InitForm();
    void showbottle();

};

#endif // FRMSAMPLER_H
