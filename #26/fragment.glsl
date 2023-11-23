#version 330 core
in vec3 FragPos; //--- ��ġ��
in vec3 Normal; //--- ���ؽ� ���̴����� ���� ��ְ�
out vec4 FragColor; //--- ���� ��ü�� �� ����

uniform vec3 lightPos; //--- ������ ��ġ
uniform vec3 lightColor; //--- ������ ��
uniform vec3 objectColor; //--- ��ü�� ��

void main ()
{
	float ambientLight = 0.3f; //--- �ֺ� ���� ���
	vec3 ambient = ambientLight * lightColor * objectColor; //--- �ֺ� ���� ��

	vec3 normalVector = normalize(Normal); //--- �븻���� ����ȭ�Ѵ�.
	vec3 lightDir = normalize(lightPos - FragPos); //--- ǥ��� ������ ��ġ�� ������ ������ �����Ѵ�.

	float diffuseLight = max (dot (normalVector, lightDir), 0.0); //--- N�� L�� ���� ������ ���� ���� (���� ���� ���� �� ���� �Ѵ�.)
	
	vec3 diffuse = diffuseLight * lightColor * objectColor; //--- ����ݻ�����=����ݻ簪*�������
	
	vec3 result = ambient + diffuse; //--- ���� ���� ������ �ȼ� ����=(�ֺ�����+����ݻ�����)*��ü ����
	FragColor = vec4 (result, 1.0); //--- �ȼ� ���� ���
}