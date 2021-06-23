#ifndef LITTLEPHOTOSHOP_BASECOMMAND_H
#define LITTLEPHOTOSHOP_BASECOMMAND_H


#include "Params.h"
#include "../Model/Model.h"
#include <QDebug>
class ViewModel;

class BaseCommand {
public:
    BaseCommand(const shared_ptr<Model> &pModel, const shared_ptr<ViewModel> &pViewModel = nullptr) : pModel(pModel),pViewModel(pViewModel) {}

    void setParams(const Params &params) {
        BaseCommand::params = params;
    }

    virtual void exec() = 0;

protected:
    Params params;
    shared_ptr<Model> pModel;
    shared_ptr<ViewModel> pViewModel;
};


#endif //LITTLEPHOTOSHOP_BASECOMMAND_H
