#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
SOAP调试工具 - 显示详细的请求和响应信息
"""

import requests
import base64
import sys
from xml.etree import ElementTree as ET

def debug_soap_request():
    """调试SOAP请求"""
    
    # SOAP配置
    soap_url = "http://127.0.0.1:7878"
    admin_user = "admin"
    admin_pass = "admin"
    
    # 测试用户名和密码
    test_username = "testuser"
    test_password = "testpass"
    
    # 构建SOAP请求
    soap_body = f'''<?xml version="1.0" encoding="UTF-8"?>
<SOAP-ENV:Envelope 
    xmlns:SOAP-ENV="http://schemas.xmlsoap.org/soap/envelope/" 
    xmlns:ns1="urn:AC">
    <SOAP-ENV:Body>
        <ns1:executeCommand>
            <command>.account create {test_username} {test_password}</command>
        </ns1:executeCommand>
    </SOAP-ENV:Body>
</SOAP-ENV:Envelope>'''

    # 设置HTTP头
    headers = {
        'Content-Type': 'text/xml; charset=utf-8',
        'SOAPAction': 'urn:AC#executeCommand',
        'Authorization': f'Basic {base64.b64encode(f"{admin_user}:{admin_pass}".encode()).decode()}'
    }
    
    print("=== SOAP调试工具 ===\n")
    print(f"URL: {soap_url}")
    print(f"管理员账号: {admin_user}")
    print(f"测试账号: {test_username}")
    print("\n=== 请求头 ===")
    for key, value in headers.items():
        print(f"{key}: {value}")
    
    print("\n=== 请求体 ===")
    print(soap_body)
    
    try:
        print("\n=== 发送请求 ===")
        response = requests.post(soap_url, data=soap_body, headers=headers, timeout=10)
        
        print(f"HTTP状态码: {response.status_code}")
        print(f"响应头: {dict(response.headers)}")
        
        print("\n=== 原始响应 ===")
        print(repr(response.text))
        
        print("\n=== 格式化响应 ===")
        print(response.text)
        
        if response.status_code == 200:
            try:
                # 尝试解析XML
                print("\n=== XML解析 ===")
                root = ET.fromstring(response.text)
                print("XML解析成功")
                
                # 查找结果元素
                print("\n=== 查找结果元素 ===")
                
                # 尝试不同的命名空间
                namespaces = {
                    'soap': 'http://schemas.xmlsoap.org/soap/envelope/',
                    'ns1': 'urn:AC'
                }
                
                # 方法1：使用命名空间
                result_element = root.find('.//ns1:executeCommandResponse/ns1:result', namespaces)
                if result_element is not None:
                    print(f"找到结果元素 (方法1): {result_element.text}")
                else:
                    print("方法1：未找到结果元素")
                
                # 方法2：不使用命名空间
                result_element2 = root.find('.//{urn:AC}executeCommandResponse/{urn:AC}result')
                if result_element2 is not None:
                    print(f"找到结果元素 (方法2): {result_element2.text}")
                else:
                    print("方法2：未找到结果元素")
                
                # 方法3：搜索所有元素
                print("\n=== 所有XML元素 ===")
                for elem in root.iter():
                    print(f"标签: {elem.tag}, 文本: {elem.text}, 属性: {elem.attrib}")
                    
            except ET.ParseError as e:
                print(f"XML解析失败: {e}")
                
        else:
            print(f"HTTP错误: {response.status_code}")
            
    except requests.exceptions.ConnectionError:
        print("❌ 连接错误：无法连接到SOAP服务")
    except requests.exceptions.Timeout:
        print("❌ 请求超时")
    except Exception as e:
        print(f"❌ 未知错误: {e}")

if __name__ == "__main__":
    debug_soap_request() 