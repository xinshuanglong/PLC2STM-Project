#include "kalmanFilter.h"

void kalmanParamInit(KFP *kfp, float P, float Q, float R)
{
    kfp->LastP = P; //�ϴι���Э����
    kfp->Now_P = 0; //��ǰ����Э���� ��ʼ��ֵΪ0
    kfp->out = 0;   //�������˲������ ��ʼ��ֵΪ0
    kfp->Kg = 0;    //����������
    kfp->Q = Q;     //��������Э���� Խ��Խ���Ų���ֵ
    kfp->R = R;     //�۲�����Э����
}

/**
*�������˲���
*@param KFP *kfp �������ṹ�����
*   float input ��Ҫ�˲��Ĳ����Ĳ���ֵ�����������Ĳɼ�ֵ��
*@return �˲���Ĳ���������ֵ��
*/
float kalmanFilter(KFP *kfp, float input)
{
    //Ԥ��Э����̣�kʱ��ϵͳ����Э���� = k-1ʱ�̵�ϵͳЭ���� + ��������Э����
    kfp->Now_P = kfp->LastP + kfp->Q;
    //���������淽�̣����������� = kʱ��ϵͳ����Э���� / ��kʱ��ϵͳ����Э���� + �۲�����Э���
    kfp->Kg = kfp->Now_P / (kfp->Now_P + kfp->R);
    //��������ֵ���̣�kʱ��״̬����������ֵ = ״̬������Ԥ��ֵ + ���������� * ������ֵ - ״̬������Ԥ��ֵ��
    kfp->out = kfp->out + kfp->Kg * (input - kfp->out); //��Ϊ��һ�ε�Ԥ��ֵ������һ�ε����
    //����Э�����: ���ε�ϵͳЭ����� kfp->LastP ����һ������׼����
    kfp->LastP = (1 - kfp->Kg) * kfp->Now_P;
    return kfp->out;
}
