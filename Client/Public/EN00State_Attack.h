#pragma once
#include "AIState_Base.h"

BEGIN(Client)

class CEN00State_Attack final : public CAIState_Base
{

public:
	BrainTree::Node::Status update() override;

};

END

