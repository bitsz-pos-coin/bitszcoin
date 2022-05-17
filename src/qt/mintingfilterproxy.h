// Copyright (c) 2012-2022 The BITSZ developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#ifndef BITSZ_QT_MINTINGFILTERPROXY_H
#define BITSZ_QT_MINTINGFILTERPROXY_H

#include <QSortFilterProxyModel>

class StakingFilterProxy : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    explicit StakingFilterProxy(QObject *parent = 0);
};

#endif // BITSZ_QT_MINTINGFILTERPROXY_H
