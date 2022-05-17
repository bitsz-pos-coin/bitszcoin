// Copyright (c) 2012-2022 The BITSZ developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#ifndef BITSZ_QT_MINTINGVIEW_H
#define BITSZ_QT_MINTINGVIEW_H

#include <QWidget>
#include <QComboBox>
#include <qt/mintingfilterproxy.h>

class WalletModel;


QT_BEGIN_NAMESPACE
class QTableView;
class QMenu;
QT_END_NAMESPACE

class StakingView : public QWidget
{
    Q_OBJECT
public:
    explicit StakingView(QWidget *parent = 0);
    void setModel(WalletModel *model);

    enum StakingEnum
    {
        Staking10min,
        Staking1day,
        Staking30days,
        Staking90days
    };

private:
    WalletModel *model;
    QTableView *mintingView;
    QComboBox *mintingCombo;
    StakingFilterProxy *mintingProxyModel;
    QMenu *contextMenu;

private Q_SLOTS:
    void contextualMenu(const QPoint &);
    void copyAddress();
    void copyTransactionId();

Q_SIGNALS:

public Q_SLOTS:
    void exportClicked();
    void chooseStakingInterval(int idx);
};

#endif // BITSZ_QT_MINTINGVIEW_H
