#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
直接在数据库中创建普通玩家账号
"""

import hashlib
import secrets
import mysql.connector

def create_player_account(username, password, host='127.0.0.1', port=3306, 
                         db_user='root', db_pass='password'):
    """创建普通玩家账号"""
    
    def calculate_srp6_values(username, password):
        """计算SRP6值"""
        N = int('894B645E89E1535BBDAD5B8B290650530801B18EBFBF5E8FAB3C82872A3E9BB7', 16)
        g = 7
        
        identity = f"{username.upper()}:{password.upper()}"
        identity_hash = hashlib.sha1(identity.encode()).digest()
        salt = secrets.token_bytes(32)
        x_hash = hashlib.sha1(salt + identity_hash).digest()
        x = int.from_bytes(x_hash, 'little') % N
        verifier = pow(g, x, N)
        verifier_bytes = verifier.to_bytes(32, 'little')
        
        return salt, verifier_bytes
    
    try:
        conn = mysql.connector.connect(
            host=host, port=port, user=db_user, password=db_pass, database='acore_auth'
        )
        cursor = conn.cursor()
        
        # 检查账号是否存在
        cursor.execute("SELECT id FROM account WHERE username = %s", (username,))
        if cursor.fetchone():
            print(f"❌ 账号 '{username}' 已存在")
            return False
        
        # 计算SRP6值
        salt, verifier = calculate_srp6_values(username, password)
        
        # 创建账号
        cursor.execute("""
            INSERT INTO account (username, salt, verifier, expansion) 
            VALUES (%s, %s, %s, 2)
        """, (username, salt, verifier))
        
        conn.commit()
        print(f"✅ 玩家账号 '{username}' 创建成功！")
        print(f"用户名: {username}")
        print(f"密码: {password}")
        return True
        
    except Exception as e:
        print(f"❌ 错误: {e}")
        return False
    finally:
        if 'conn' in locals() and conn.is_connected():
            cursor.close()
            conn.close()

if __name__ == "__main__":
    print("=== 创建普通玩家账号 ===\n")
    username = input("玩家用户名: ").strip()
    password = input("玩家密码: ").strip()
    
    if username and password:
        create_player_account(username, password)
    else:
        print("❌ 用户名和密码不能为空") 