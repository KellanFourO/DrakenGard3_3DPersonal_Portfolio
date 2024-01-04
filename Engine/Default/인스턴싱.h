#pragma once

//TODO 인스턴싱 이란?


//! 인스턴싱은 같은 모델을 여러번 반복해서 찍어야 할때 사용할 수 있는 기법 중 하나이다.

//! 파티클로 예를 들시 네모를 여러 개 그려서 이펙트를 만들어가는 것을 파티클이라고 부른다.
//! 위 설명과 같이 파티클로 이펙트를 만들려면 네모를 여러 개 그려야하는 작업이 필요하다고 했다. 
//! 파티클은 많게는 3 ~ 4000개 까지 갈 수 있기에 최적화가 필요했다.

//!  동일한 파티클(눈, 비)을 3000개 그린다고 가정할 경우, 중복되는 행동들이 3천번 일어난다 . 
//! 눈이나 비 같은 경우에는 같은 이미지를 사용하는게 대 다수이며, 같은 정점 버퍼와 인덱스 버퍼, 같은 셰이더를 사용할 것이다.
//! 월드행렬은 각자 다른 것을 쓰겠지만 뷰와 투영행렬도 같은 걸 알 수 있다
//! 버퍼와 인덱스 바인딩도 3천번을 해야하며 셰이더의 비긴과 앤드도 3천번을 해야하는데 이 드로우 콜수를 줄이기 위해 
//!  버퍼와 인덱스 바인딩을 한번만 수행하고, 셰이더도 한번만 비긴과 앤드를 수행하며.
//!  그리는 위치만 바꿔서 3천개를 그릴 수 있게 만들어주는 것이 인스턴싱이다.

/*TODO 정리하자면*/ //! 한번에 다 처리하자라는 개념이고.
				//! 중복된 데이터, 중복된 자원을 이용해서 그려진 다수의 개체, 모델들이 존재한다라면 같은 것을 반복적으로 처리하지 말고 한번의 한번 드로우콜로 다수의 개체들을 그려내자 라는 기법

//TODO 즉, 인스턴싱을 하는 이유 : 쓸데없는 드로우 콜을 줄이기 위해서.

//! 응용 할 시에는 애니메이션 모델들도 인스턴싱할 수 있다고 한다. ( 디아블로, 핵앤슬래시 몬스터들이 수없이 많이 나와서 전투하는 경우 )

//TODO 사용방법. 
//! 파티클(눈)을 2개 그린다고 해보자. 눈은 사각 버퍼를 사용하기에 눈 1개당 정점은 4개이며 인덱스는 6개이다. ( 0 , 1, 2 ), (0, 2, 3)
//! 인스턴싱을 이용해서 눈을 2개 그린다면 정점 4개는 그대로 두며 인덱스는 6 * 그릴 개수가 되서 12개가 되는 것.
//! 이대로만 그린다면 눈을 2개 그린다는 행위 자체는 맞지만 겹쳐져서 그려지므로 1개처럼 보일 것이다.
//! 그전에 우리가 일반 객체들을 그릴 때는 각기 월드행렬을 다르게 줘서 다른 위치에 그린다는 방법으로 접근했었다.
//! 그 방법처럼 2개의 눈들에게 각각 다른 위치를 표현하는 상태행렬이란 개념이 도입된 것.
//! 상태행렬은 로컬 스페이스상에서 네모들의 위치, 움직임을 가져가는 걸 취해주자.
//! 눈을 2개 그릴 것이니 인덱스는 12개, 상태행렬은 2개, 인스턴스 버퍼도 2개
//! 정점들에게 이 상태행렬을 곱해서 표현해야 하는데, 이 행위는 셰이더에서 처리할 것이다.
//! 모델 하나를 그릴 때, 우리는 셰이더 전역변수에 값을 던진다라는 행위를 했었다.
//! DX는 인스턴싱에서 사용하는 개념의 행위를 쉽게 할 수 있또록 버텍스 버퍼를 여러개 장치에 바인딩 하는 기능을 제공했다.
//! 인스턴스 버퍼는 float 타입을 4개 들고있는 _float4 타입을 4개 들게 해줘서 총 16개의 float을 가지게 할 것이며 이 16개의 float은 정점이자  사실 행렬이다.
//! 셰이더에게 던져줄 때 버퍼 한개의 정점이 던져지는 것이아닌, 바인딩 된 모든 정점들이 던져진다.

//! 정점 몇개마다 하나씩 던질 것이냐는 설정을 우리가 해줄 수 있다
/*! 렌더할 때 드로우 인덱스드가 아닌 드로우스인덱스드인스턴스드 *//*TODODrawIndexedInstanced*/ //! 함수를 이용하여 그리기위한 인스턴스 버퍼를 추가로 사용하고있다는 것을 명시적으로 장치에게 알린다
