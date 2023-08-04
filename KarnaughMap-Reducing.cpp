#include <iostream>
#include <string>
#include <vector>
#include <cstring>
#include <algorithm>
#include <cmath>
#include <set>
#include <cstdlib>
#include <conio.h>
using namespace std;

void showbit(unsigned int a,int n);
int countBits(int n);//函数声明
vector<vector<int> > all_cb(const vector<int>&input,int n);
bool have_same_element(const vector<int>&a,const vector<int>&b);
inline bool judgebit_r(int in,int n);

class kmap//卡诺图
{
public:
	vector<unsigned int>truths;//原始真值坐标
	void printkmap(void);
	void process(void);
	kmap(unsigned int d,unsigned int n)//构造函数，d对应dimensions属性，n为真值个数
	{
		dimensions=d;//将输入的d设置为对象的dimensions属性
		iscp=false;//一开始是没比较过的
		truths.resize(n);//将存储原始真值坐标的向量初始化
		incl_index.resize(n);
		circles.resize(2);
	}
private:
	unsigned int dimensions;//变量个数or维度数
	bool iscp;//是否调用过compare方法
	vector<vector<unsigned int> > circles;//在卡诺图上画的圈坐标，[0][...]记录0、1部分，[1][...]记录2部分
	vector<int>ess_cir_index;//所有必要质蕴涵项在circles[0/1][]中的索引
	vector<vector<int> >incl_index;
	vector<vector<int> >incl_truths;//和circles[0/1][]对应，存储该圈包含哪些真值
	vector<vector<int> >need2incl_index;
	vector<vector<int> >fi_res;
	vector<string>clauses;
	void compare(void);//声明比较函数
	void get_ess_cir(void);
	void find_inneed(void);
	bool pre_fi(void);
	bool check(const vector<int>&cb_list);
	void cir2clause(void);
};//结束kmap类


void kmap::compare(void)//用于查找相邻的真值坐标或圈坐标，并合并到circles属性
{
	int i,j,xorr,xorc,pushtmp1,pushtmp2;//xorr:z综合异或结果
	vector<vector<unsigned int> >tmp(2);//临时缓存，随后通过memcpy复制到circles属性
	vector<unsigned int>::iterator check;
	vector<bool>needpush;//是否要再次推入(**取反**)
	if(iscp)
	{
		needpush.resize(circles[0].size());//设置项数
		for(i=0;i<circles[0].size()-1;i++)
		{
			for(j=i+1;j<circles[0].size();j++)
			{
				xorr=(circles[0][i]^circles[0][j])|(circles[1][i]^circles[1][j]);
				xorc=countBits(xorr);
				if(xorc<2)
				{
					pushtmp1=circles[0][i]|xorr;
					pushtmp2=circles[1][i]|xorr;
					check=find(tmp[0].begin(),tmp[0].end(),pushtmp1);
					if(!((check!=tmp[0].end())&&(tmp[1][check-tmp[0].begin()]==pushtmp2)))
					{
						tmp[0].push_back(pushtmp1);
						tmp[1].push_back(pushtmp2);
					}
					needpush[i]=true;
					needpush[j]=true;
				}
			}
		}
		for(i=0;i<circles[0].size();i++)
		{
			if(!needpush[i])
			{
				tmp[0].push_back(circles[0][i]);
				tmp[1].push_back(circles[1][i]);
			}
		}
		tmp.swap(circles);
	}
	else//没比较过，只用比较truths属性的0、1部分
	{
		needpush.resize(truths.size());//设置项数
		for(i=0;i<truths.size()-1;i++)
		{
			for(j=i+1;j<truths.size();j++)
			{
				xorr=truths[i]^truths[j];
				xorc=countBits(xorr);
				if(xorc<2)
				{
					pushtmp1=truths[i]|xorr;
					pushtmp2=xorr;//实际上为0|xorr
					tmp[0].push_back(pushtmp1);
					tmp[1].push_back(pushtmp2);
					needpush[i]=true;
					needpush[j]=true;
				}
			}
		}
		for(i=0;i<truths.size();i++)
		{
			if(!needpush[i])
			{
				tmp[0].push_back(truths[i]);
				tmp[1].push_back(0);
			}
		}
		tmp.swap(circles);
	}
	iscp=true;
}


void kmap::printkmap(void)//打印卡诺图中所有画的圈，若尚未画圈，则输出所有原始真值坐标
{
	int i;
	cout<<"The KMAP contains "<<dimensions<<" dimensions"<<endl;
	if(iscp)
	{
		for(i=0;i<circles[0].size();i++)
		{
			showbit(circles[0][i],dimensions);
			cout<<"  ";
		}
		cout<<"\n";
		for(i=0;i<circles[1].size();i++)
		{
			showbit(circles[1][i],dimensions);
			cout<<"  ";
		}
	}
	else
	{
		cout<<"Original:"<<endl;
		for(i=0;i<truths.size();i++)
		{
			showbit(truths[i],dimensions);
			cout<<"  ";
		}
	}
	cout<<"\n\n";
}


void kmap::get_ess_cir(void)//统计所有必要质蕴涵项，返回它们在circles[0/1][]中的所有索引
{
	int i,j;
	incl_truths.resize(circles[0].size());
	//vector<vector<int> >incl_index(truths.size());//存储每个真值分别被哪些索引对应的circles[...]所包含
	for(i=0;i<circles[0].size();i++)
	{
		for(j=0;j<truths.size();j++)
		{
			if((circles[0][i]^(truths[j]|circles[1][i]))==0)//判断circles[...][i]是否包含truths[j]
			{
				incl_index[j].push_back(i);//若包含，则将该circle的索引存入truths[j]对应的incl_index中
				incl_truths[i].push_back(j);
			}
		}
	}
	//此时已经分析完所有圈分别包含哪些truths(真值)(最小项)
	//随后挑出只被一个圈包含的truths
	//对应的圈即为“必要质蕴涵项”
	for(i=0;i<truths.size();i++)//遍历所有truths(真值)(最小项)
	{
		if(incl_index[i].size()==1)//其实等价于==1
		{
			ess_cir_index.push_back(incl_index[i][0]);//既然这些必要最小项只被一个circle包含，也就说incl_index中只存有一个索引
		}
	}
	sort(ess_cir_index.begin(),ess_cir_index.end());//去重
	ess_cir_index.erase(unique(ess_cir_index.begin(),ess_cir_index.end()),ess_cir_index.end());
}

//判断“必要质蕴涵项”是否覆盖所有真值，并返回判断结果：
//若是：则无需进行find_ineed
//若否：则设置need2incl_index为“必要质蕴涵项”还未填充的真值所对应的所有“能填充这个真值的圈”在circles中的索引
bool kmap::pre_fi(void)
{
	int i,j;
	vector<int>check_truths_index;
	vector<bool>truth_isincl(truths.size());//一一对应truths，存储该真值是否被必要质蕴涵项包含
	for(i=0;i<ess_cir_index.size();i++)
	{
		check_truths_index.insert(check_truths_index.end(),incl_truths[ess_cir_index[i]].begin(),incl_truths[ess_cir_index[i]].end());
	}
	//此时check_truths_index存放了所有必要质蕴涵项所包含的真值索引，但仍有重复，需要去重(采用sort+unique+erase)
	sort(check_truths_index.begin(),check_truths_index.end());
	check_truths_index.erase(unique(check_truths_index.begin(),check_truths_index.end()),check_truths_index.end());
	if(check_truths_index.size()==truths.size())
	{
		return true;
	}
	else
	{
		for(i=0;i<check_truths_index.size();i++)
		{
			truth_isincl[check_truths_index[i]]=true;
		}
		for(i=0;i<incl_index.size();i++)
		{
			if(!truth_isincl[i])
			{
				need2incl_index.push_back(incl_index[i]);
			}
		}
		return false;
	}
}


void kmap::find_inneed(void)//找出所有可能的结果，并存入fi_res
{
	int i,j;
	bool found=false;
	vector<int>ncb_cir_list;//需要进行组合的圈(非必要质蕴涵项)
	vector<vector<int> >cb_list;
	for(i=0;i<need2incl_index.size();i++)
	{
		ncb_cir_list.insert(ncb_cir_list.end(),need2incl_index[i].begin(),need2incl_index[i].end());
	}
	sort(ncb_cir_list.begin(),ncb_cir_list.end());
	ncb_cir_list.erase(unique(ncb_cir_list.begin(),ncb_cir_list.end()),ncb_cir_list.end());
	for(i=1;i<=ncb_cir_list.size();i++)//从1开始
	{
		cb_list=all_cb(ncb_cir_list,i);
		for(j=0;j<cb_list.size();j++)//遍历每一种排列情况
		{
			if(check(cb_list[j]))
			{
				cb_list[j].insert(cb_list[j].begin(),ess_cir_index.begin(),ess_cir_index.end());
				sort(cb_list[j].begin(),cb_list[j].end());
				fi_res.push_back(cb_list[j]);
				found=true;
			}
		}
		if(found)
		{
			break;
		}
	}
}


bool kmap::check(const vector<int>&cb_list)//输入一种非必要蕴涵项的组合方式，判断是否能完整填充所需的真值(这些真值尚未被覆盖)
{
	int i;
	for(i=0;i<need2incl_index.size();i++)
	{
		if(!have_same_element(cb_list,need2incl_index[i]))
			return false;
	}
	return true;
}


void kmap::cir2clause(void)
{
	int i,j,k;
	char alphabet;
	string cl_tmp;
	for(i=0;i<fi_res.size();i++)//每一种情况:fi_res[]
	{
		cl_tmp="";
		for(j=0;j<fi_res[i].size();j++)//每一个圈:索引为fi_res[][]的circles[0/1][]
		{
			//圈的每一位坐标:circles[0/1][fi_res[][]]的每一个二进制位
			cl_tmp.append("(");
			for(k=0,alphabet='A';k<dimensions;k++,alphabet++)
			{
				if(judgebit_r(circles[1][fi_res[i][j]],dimensions-k))//从前到后d-...-4-3-2-1
					continue;
				if(cl_tmp[cl_tmp.length()-1]!='(')
					cl_tmp.append("&&");
				if(judgebit_r(circles[0][fi_res[i][j]],dimensions-k))
					cl_tmp.append(1,alphabet);
				else
				{
					cl_tmp.append("!");
					cl_tmp.append(1,alphabet);
				}
			}
			cl_tmp.append(")");
			if(j!=fi_res[i].size()-1)
				cl_tmp.append("||");
		}
		clauses.push_back(cl_tmp);
	}
}


void kmap::process(void)
{
	int i,j;
	if(truths.size()==0)
	{
		cout<<"0"<<endl;//卡诺图全0:化简结果为0
		return;
	}
	if(truths.size()==round(pow(2.0,dimensions)))
	{
		cout<<"1"<<endl;//卡诺图全1:化简结果为1
		return;
	}
	for(i=0;i<dimensions;i++)
	{
		compare();
		printkmap();
	}
	get_ess_cir();
	cout<<"essentials'indexes: ";
	for(i=0;i<ess_cir_index.size();i++)
	{
		cout<<ess_cir_index[i]<<" ";
	}
	cout<<"\n\n";
	bool fi=pre_fi();
	cout<<"fi="<<fi<<endl;
	if(need2incl_index.begin()==need2incl_index.end())
	{
		cout<<"warning: need2incl_index is EMPTY!!"<<endl;
		fi_res.push_back(ess_cir_index);
	}
	else
	{
		for(i=0;i<need2incl_index.size();i++)
		{
			cout<<"n2i["<<i<<"]=";
			for(j=0;j<need2incl_index[i].size();j++)
			{
				cout<<need2incl_index[i][j]<<" ";
			}
			cout<<"\n";
		}
		cout<<"\n";
		find_inneed();
	}
	for(i=0;i<fi_res.size();i++)
	{
		cout<<"fi_res["<<i<<"]=";
		for(j=0;j<fi_res[i].size();j++)
		{
			cout<<fi_res[i][j]<<" ";
		}
		cout<<"\n";
	}
	cir2clause();
	cout<<"\n***化简结果***:"<<endl;
	for(i=0;i<clauses.size();i++)
	{
		cout<<clauses[i]<<endl;
	}
	cout<<"\n程序终止\n"<<endl;
}


int main()
{
	int i,d,n;
	char open_website1,open_website2;
	system("chcp 65001");
	cout<<"\n你好！这是一个化简逻辑函数的程序，由Chesium编写"<<endl
		<<"它采用卡诺图化简法来化简表达式"<<endl
		<<"如果你不知道卡诺图是什么，现在按下[h]键可以打开\"卡诺图化简法\"的百度百科词条(按其他按键以继续): ";
	open_website1=getch();
	if(open_website1=='h')
		system("start https://baike.baidu.com/item/%E5%8D%A1%E8%AF%BA%E5%9B%BE%E5%8C%96%E7%AE%80%E6%B3%95");
	cout<<"\n\n好，我们开始吧：先输入你要化简的函数包含的变量数d(最好不大于26，以保证用A~Z的字母来显示结果)"<<endl;
	ENTER_D:cout<<"请输入一个正整数d: ";
	cin>>d;
	if(d>26)
		cout<<"太大了！乱码警告！"<<endl;
	if(d<=0)
	{
		cout<<"你都在输些什么呢?重输！"<<endl;
		goto ENTER_D; 
	}
	cout<<"\n这个程序无需知道你的函数或表达式到底是什么，你只需要提供你的函数或表达式结果为1(真)时各个变量的取值，这可能有好几种情况"<<endl
		<<"按下[h]键以显示示例(按其他按键以跳过): ";
	open_website2=getch();
	cout<<open_website2<<endl;
	if(open_website2=='h')
		cout<<"\n\n    示例："<<endl
			<<"        对于函数 (A&B&C)|(A&B&!C)|(A&!B&C)|(!A&B&C)   (其中\"&\"意为\"逻辑与\"，\"|\"意为\"逻辑或\"，\"!\"意为\"逻辑非\")"<<endl
			<<"        有且仅有在：\n"<<endl
			<<"            1. A为真  B为真  C为真  对应二进制为111，转换为十进制为7"<<endl
			<<"            2. A为真  B为真  C为假  对应二进制为110，转换为十进制为6"<<endl
			<<"            3. A为真  B为假  C为真  对应二进制为101，转换为十进制为5"<<endl
			<<"            4. A为假  B为真  C为真  对应二进制为011，转换为十进制为3\n"<<endl
			<<"        四种情况时函数最终取值为\"真\"，其他情况取值都为\"假\""<<endl
			<<"        所以按照我的设计，接下来首先要输入这些取“真”情况的个数，这里为4"<<endl
			<<"        然后输入这些情况所对应的十进制数：7、6、5、3，不需要按顺序，两个数之间要按回车，即可";
	ENTER_N:cout<<"\n\n请输入函数取值为真的情况个数n，应为不大于2^d的自然数： ";
	cin>>n;
	if(n<0)
	{
		cout<<"你都在输些什么呢?重输！"<<endl;
		goto ENTER_N; 
	}
	if(n==0)
	{
		cout<<"检测到你输入了0，那化简结果就是0(滑稽)"<<endl;
		return 0;
	}
	if(n==round(pow(2.0,d)))
	{
		cout<<"看上去你的函数不管输入啥输出都为1，那么最终的化简结果就是1(滑稽)"<<endl;
		return 0;
	}
	cout<<"一切顺利，接下来输入此函数这"<<n<<"种取真值的情况对应的十进制形式，不明白请查看帮助或联系作者";
	kmap yourkmap(d,n);
	cout<<"\n";
	for(i=0;i<n;i++)
	{
		cout<<"请输入第"<<i+1<<"种情况: ";
		cin>>yourkmap.truths[i];
	}
	cout<<"\n\n输入完成。调用printkmap函数打印此卡诺图:"<<endl;
	yourkmap.printkmap();
	cout<<"按任意键开始计算\n";
	open_website1=getch();
	yourkmap.process();
	system("pause");
	return 0;
}//结束main函数

void showbit(unsigned int a,int n)//以二进制形式打印无符号整数，若结果位数小于n，则补"0"至n位处
{
	string ans="";
	int c=0;
	while(a)
	{
		++c;
		ans.insert(0,1,a%2+'0');
		a=a/2;
	}
	if(c<n)
		ans.insert(0,n-c,'0');
	cout<<ans;
}


int countBits(int n)//检索一个整数的二进制形式中有多少个"1"
{
	int count=0;
	while(n!=0)
	{
		n=n&(n-1);
		count++;
	}
	return count;
}


vector<vector<int> > all_cb(const vector<int>&input,int n)
{
	int i,j,er_t;
	vector<vector<int> >results;
	vector<vector<int> >final;
	vector<int>tmp;
	vector<int>in_copy;
	vector<int>::iterator in_it;
	if(n==1)
	{
		for(i=0;i<input.size();i++)
		{
			tmp.clear();
			tmp.push_back(input[i]);
			results.push_back(tmp);
		}
		return results;
	}
	else
	{
		for(i=0;i<input.size()-1;i++)
		{
			in_copy=input;
			er_t=in_copy[i];
			in_it=in_copy.begin()+i;
			in_copy.erase(in_copy.begin(),in_it+1);
			results=all_cb(in_copy,n-1);
			for(j=0;j<results.size();j++)
			{
				results[j].push_back(er_t);
				final.push_back(results[j]);
			}
		}
		return final;
	}
}


bool have_same_element(const vector<int>&a,const vector<int>&b)
{
	set<int>m(a.begin(),a.end());
	vector<int>::const_iterator it=b.begin();
	for(;it!=b.end();it++)
		if(m.find(*it)!=m.end())
			return true;
	return false;
}


inline bool judgebit_r(int in,int n)//从右到左
{
	return in>>n-1&1;
}