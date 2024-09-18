#pragma once



class DBProcessContainer : public IContainer<DBPROCESS_CER_PTR>, public Singleton<DBProcessContainer>
{
public:

	DBProcessContainer(void);
	~DBProcessContainer(void);

	virtual bool Add(DBPROCESS_CER_PTR &pObj);
	virtual bool Del(DBPROCESS_CER_PTR &pObj);
	virtual DBPROCESS_CER_PTR Search(INT64 Id);

	//�α��ξƿ� ó���� �����ؾ��� �ʿ䰡 �ִ�.
	//id�� ���� ���ҵǸ� ��Ƽ ������ ����ȭ ������ �߻��� ���ɼ��� �����Ѵ�.
	DBPROCESS_CER_PTR GetFirstHandle();

	void TryConnect();

	void Initialize(INT Count);

	std::map<DWORD,DBPROCESS_CER_PTR> m_List;
};

#define DBPROCESSCONTAINER_CER Singleton<DBProcessContainer>::Instance()