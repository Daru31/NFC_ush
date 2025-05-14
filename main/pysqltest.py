import pymysql 
 
def dbconnect() : 
    conn = pymysql.connect (host='221.152.204.73',
                            user ='ushtech',
                            password='a12345678',
                            db='student_db',
                            charset='utf8')
    return conn 

def search_data(conn):
    cur = conn.cursor()
    sql = 'SELECT number FROM student_info'
    cur.execute(sql)
    results = cur.fetchall()
    print(results)
    
def main():
    conn = dbconnect()  # DB 연결
    print('연결완료')
    search_data(conn)
    
    
if __name__=="__main__" :
    main()