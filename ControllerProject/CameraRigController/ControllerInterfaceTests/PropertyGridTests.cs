using Microsoft.VisualStudio.TestTools.UnitTesting;
using System;
using FluentAssertions;
using CameraRigController.FieldGrid;
using System.ComponentModel;
using System.Linq;
using CameraRigController.FieldGrid.Editor.ViewModel;
using CameraRigController.FieldGrid;

namespace ControllerInterfaceTests
{
    [TestClass]
    public class PropertyGridTests
    {
        [DataTestMethod]
        [DataRow("ValueType", "Value type")]
        [DataRow("RGBColor", "RGB color")]
        [DataRow("Value1", "Value 1")]
        [DataRow("Value1b", "Value 1b")]
        [DataRow("Value1B", "Value 1 b")]
        [DataRow("Value15", "Value 15")]
        [DataRow("Value15b", "Value 15b")]
        [DataRow("Value15B", "Value 15 b")]
        [DataRow("Value15RGB", "Value 15 RGB")]
        [DataRow("Value15RGBGood1bTo5a", "Value 15 RGB good 1b to 5a")]
        [DataRow("MGHello24World", "MG hello 24 world")]

        public void TestNicifyNames(string value, string expected)
        {
            FieldGridUtillities.NicifyName(value).Should().Be(expected);
        }

        [TestMethod]
        public void TestSupportedTypes()
        {
            var t = new Type[]
            {
                typeof(string),
            };

            foreach (var type in t)
            {
                FieldGridUtillities.IsSupported(type);
            }
        }

        class DemoObject1 : INotifyPropertyChanged
        {
            private string _value1;

            public string Value1
            {
                get => _value1;
                set
                {
                    _value1 = value;
                    OnPropertyChanged(nameof(Value1));
                }
            }

            private string _value2;
            [ReadOnly(true)]
            public string Value2
            {
                get { return _value2; }
                set 
                { 
                    _value2 = value;
                    OnPropertyChanged(nameof(Value2));
                }
            }

            private int _value3;

            public int Value3
            {
                get => _value3;
                set
                {
                    _value3 = value;
                    OnPropertyChanged(nameof(Value3));
                }
            }

            private int _value4;
            
            [Slider(0, 10)]
            public int Value4
            {
                get => _value4;
                set
                {
                    _value4 = value;
                    OnPropertyChanged(nameof(Value4));
                }
            }

            public event PropertyChangedEventHandler PropertyChanged;

            private void OnPropertyChanged(string propertyName)
            {
                PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
            }

        }

        [TestMethod]
        public void TestStringDataConversion()
        {
            var obj = new DemoObject1() { Value1 = "Hello" };

            var collection = FieldGridUtillities.ToVMCollection(obj);
            collection[0].DisplayName.Should().Be("Value 1");
            collection[0].Should().BeOfType(typeof(StringEditorVM));
        }

        [TestMethod]
        public void TestStringDataBinding()
        {
            var obj = new DemoObject1() { Value1 = "Test1" };

            var collection = FieldGridUtillities.ToVMCollection(obj);
            var value1 = collection.First((p) => p.PropertyName == "Value1");

            value1.ObjectValue.Should().Be("Test1");

            value1.ObjectValue = "Test2";
            obj.Value1.Should().Be("Test2");

            obj.Value1 = "Test3";
            value1.ObjectValue.Should().Be("Test3");
        }

        [TestMethod]
        public void TestReadonlyStringDataConversion()
        {
            var obj = new DemoObject1() { Value2 = "Hello" };

            var collection = FieldGridUtillities.ToVMCollection(obj);
            var value2 = collection.First((p) => p.PropertyName == "Value2");

            value2.Should().BeOfType<ReadonlyStringEditorVM>();
            value2.ObjectValue.Should().Be("Hello");

            obj.Value2 = "World";
            value2.ObjectValue.Should().Be("World");
        }

        [TestMethod]
        public void TestSimpleIntEditor()
        {
            var obj = new DemoObject1() { Value3 = 3 };

            var collection = FieldGridUtillities.ToVMCollection(obj);
            var value3 = collection.First((p) => p.PropertyName == "Value3");

            value3.Should().BeOfType<SimpleIntEditorVM>();
            value3.ObjectValue.Should().Be(3);

            obj.Value3 = 10;
            value3.ObjectValue.Should().Be(10);
        }

        [TestMethod]
        public void TestIntSlider()
        {
            var obj = new DemoObject1() { Value4 = 6 };

            var collection = FieldGridUtillities.ToVMCollection(obj);
            var value4 = collection.First((p) => p.PropertyName == "Value4").As<IntSliderEditorVM>();

            value4.Should().BeOfType<IntSliderEditorVM>();
            value4.Minimum.Should().Be(0);
            value4.Maximum.Should().Be(10);
            value4.ObjectValue.Should().Be(6);

            obj.Value4 = 10;
            value4.ObjectValue.Should().Be(10);
        }

        [TestMethod]
        public void TestEnumInheritance()
        {
            var e = RefreshProperties.Repaint;
            e.GetType().IsSubclassOf(typeof(Enum)).Should().BeTrue();
        }
    }
}
