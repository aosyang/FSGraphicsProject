//=============================================================================
// RDelegate.h by Shiyang Ao, 2018 All Rights Reserved.
//
// 
//=============================================================================

#pragma once

template<typename... Args>
class RDelegate
{
	struct RDelegateFuncSignature
	{
		virtual void Execute(Args... args) {}
	};

public:

	/// Bind to a function pointer
	void Bind(void(*Func)(Args...))
	{
		struct RDelegateAction : public RDelegateFuncSignature
		{
			RDelegateAction(void(*InFunc)(Args...))
				: Func(InFunc)
			{
			}

			virtual void Execute(Args... args) override
			{
				(*Func)(args...);
			}

		private:
			void(*Func)(Args...);
		};

		std::unique_ptr<RDelegateFuncSignature> DelegateAction(new RDelegateAction(Func));
		DelegateActions.push_back(std::move(DelegateAction));
	}

	/// Bind to a member function of an object 
	template<typename T>
	void Bind(T* Object, void(T::*Func)(Args...))
	{
		struct RDelegateAction : public RDelegateFuncSignature
		{
			RDelegateAction(T* InObject, void(T::*InFunc)(Args...))
				: Object(InObject)
				, Func(InFunc)
			{
			}

			virtual void Execute(Args... args) override
			{
				(Object->*Func)(args...);
			}

		private:
			T* Object;
			void(T::*Func)(Args...);
		};

		std::unique_ptr<RDelegateFuncSignature> DelegateAction(new RDelegateAction(Object, Func));
		DelegateActions.push_back(std::move(DelegateAction));
	}

	/// Bind to a lambda expression
	void BindLambda(std::function<void(Args...)> Lambda)
	{
		struct RDelegateAction : public RDelegateFuncSignature
		{
			RDelegateAction(std::function<void(Args...)> InLambda)
				: Lambda(InLambda)
			{
			}

			virtual void Execute(Args... args) override
			{
				Lambda(args...);
			}

		private:
			std::function<void(Args...)> Lambda;
		};

		std::unique_ptr<RDelegateFuncSignature> DelegateAction(new RDelegateAction(Lambda));
		DelegateActions.push_back(std::move(DelegateAction));
	}

	/// Unbind all functions from the delegate
	void UnbindAll()
	{
		DelegateActions.clear();
	}

	/// Execute all functions bound to the delegate
	void Execute(Args... args)
	{
		for (auto& Action : DelegateActions)
		{
			Action->Execute(args...);
		}
	}

private:
	std::vector<std::unique_ptr<RDelegateFuncSignature>> DelegateActions;
};
